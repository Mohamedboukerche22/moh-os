#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "mohos.h"

#define C_RESET		"\033[0m"
#define C_BOLD		"\033[1m"
#define C_CYAN		"\033[36m"

#define ART_PAD		16
#define FROW_MAX	9

#define GIB		(1024UL * 1024UL * 1024UL)

struct frow {
	char	key[24];
	char	val[96];
};

static const char *const mohos_art[] = {
	"   .--.",
	"  |o_o |",
	"  |:_/ |",
	" //   \\ \\",
	"(|     | )",
	"/'\\_   _/`\\",
	"\\___)=(___/",
};

static void frow_push(struct frow *r, int *n, const char *key,
		      const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	strncpy(r[*n].key, key, sizeof(r[*n].key) - 1);
	vsnprintf(r[*n].val, sizeof(r[*n].val), fmt, ap);
	va_end(ap);

	(*n)++;
}

static int file_first_line(const char *path, char *out, size_t len)
{
	FILE *f;
	size_t l;

	f = fopen(path, "r");
	if (!f)
		return -1;

	if (!fgets(out, len, f)) {
		fclose(f);
		return -1;
	}

	l = strlen(out);
	while (l && (out[l - 1] == '\n' || out[l - 1] == '\r'))
		out[--l] = '\0';

	fclose(f);
	return 0;
}

static int proc_read(const char *path, const char *needle,
		     char *out, size_t len)
{
	FILE *f;
	char line[256];
	char *p;
	size_t l;

	f = fopen(path, "r");
	if (!f)
		return -1;

	while (fgets(line, sizeof(line), f)) {
		if (strncmp(line, needle, strlen(needle)))
			continue;

		p = strchr(line, ':');
		if (!p)
			break;
		p++;
		while (*p == ' ' || *p == '\t')
			p++;

		strncpy(out, p, len - 1);
		out[len - 1] = '\0';
		l = strlen(out);
		if (l && out[l - 1] == '\n')
			out[l - 1] = '\0';

		fclose(f);
		return 0;
	}

	fclose(f);
	return -1;
}

static int proc_count(const char *path, const char *needle)
{
	FILE *f;
	char line[128];
	int n = 0;

	f = fopen(path, "r");
	if (!f)
		return -1;

	while (fgets(line, sizeof(line), f))
		if (!strncmp(line, needle, strlen(needle)))
			n++;

	fclose(f);
	return n;
}

static long proc_kb(const char *path, const char *needle)
{
	char buf[32];

	if (proc_read(path, needle, buf, sizeof(buf)))
		return -1;

	return strtol(buf, NULL, 10);
}

static void fetch_host_user(struct frow *r, int *n)
{
	char host[64];
	const char *user;

	if (gethostname(host, sizeof(host) - 1))
		strcpy(host, "mohos");

	user = getenv("USER");
	if (!user)
		user = "user";

	frow_push(r, n, "Host", "%s@%s", user, host);
}

static void fetch_dmi(struct frow *r, int *n)
{
	char vend[64] = "unknown";
	char prod[64] = "";

	file_first_line("/sys/class/dmi/id/sys_vendor", vend, sizeof(vend));
	file_first_line("/sys/class/dmi/id/product_name", prod, sizeof(prod));

	frow_push(r, n, "System", prod[0] ? "%s %s" : "%s", vend, prod);
}

static void fetch_cpu(struct frow *r, int *n)
{
	char model[128];
	int cores;

	if (proc_read("/proc/cpuinfo", "model name", model, sizeof(model)))
		strcpy(model, "unknown");

	cores = proc_count("/proc/cpuinfo", "processor");
	if (cores > 1)
		frow_push(r, n, "CPU", "%s (%d)", model, cores);
	else
		frow_push(r, n, "CPU", "%s", model);
}

static void fetch_gpu(struct frow *r, int *n)
{
	char line[512];
	char *p = NULL;
	FILE *f;

	f = popen("lspci 2>/dev/null | grep -iE 'vga|3d controller' | head -n 1",
		  "r");
	if (f) {
		if (fgets(line, sizeof(line), f)) {
			p = strstr(line, "controller");
			if (p) {
				p = strchr(p, ':');
				if (p) {
					p++;
					while (*p == ' ' || *p == '\t')
						p++;
				}
			} else {
				p = line;
			}
		}
		pclose(f);
	}

	frow_push(r, n, "GPU", "%s", p ? p : "unknown");
}

static void fetch_ram(struct frow *r, int *n)
{
	long tot, avail;

	tot = proc_kb("/proc/meminfo", "MemTotal");
	avail = proc_kb("/proc/meminfo", "MemAvailable");

	if (tot < 0)
		frow_push(r, n, "RAM", "unknown");
	else if (avail < 0)
		frow_push(r, n, "RAM", "%.0fMiB", tot / 1024.0);
	else
		frow_push(r, n, "RAM", "%.0fMiB / %.0fMiB",
			  avail / 1024.0, tot / 1024.0);
}

static unsigned long uptime_secs(void)
{
	FILE *f;
	char buf[64];
	unsigned long s = 0;

	f = fopen("/proc/uptime", "r");
	if (f && fgets(buf, sizeof(buf), f))
		s = strtoul(buf, NULL, 10);
	if (f)
		fclose(f);

	return s;
}

static void fetch_uptime(struct frow *r, int *n)
{
	unsigned long s = uptime_secs();
	unsigned long d, h, m;

	d = s / 86400;
	h = s % 86400 / 3600;
	m = s % 3600 / 60;

	frow_push(r, n, "Uptime", "%lud %luh %lum", d, h, m);
}

static void fetch_disk(struct frow *r, int *n)
{
	struct statvfs st;
	unsigned long total, used;

	if (statvfs("/", &st) || !st.f_frsize)
		goto unknown;

	total = st.f_frsize * st.f_blocks;
	used = total - st.f_frsize * st.f_bavail;

	frow_push(r, n, "Disk", "%luGiB / %luGiB (%lu%%)",
		  used / GIB, total / GIB, used * 100 / total);
	return;

unknown:
	frow_push(r, n, "Disk", "unknown");
}

static void fetch_render(const struct frow *r, int n)
{
	int artn = (int)ARRAY_SIZE(mohos_art);
	int i;

	for (i = 0; i < artn || i < n; i++) {
		const char *a = i < artn ? mohos_art[i] : "";
		const char *k = i < n ? r[i].key : "";
		const char *v = i < n ? r[i].val : "";

		printf(C_CYAN "%-*s" C_RESET C_CYAN "%s" C_RESET
		       C_BOLD ": %s" C_RESET "\n",
		       ART_PAD, a, k, v);
	}
}

static int do_mohfetch(struct shell *sh, int argc, char **argv)
{
	struct frow rows[FROW_MAX];
	struct utsname un;
	int n = 0;

	(void)sh;
	(void)argc;
	(void)argv;

	fetch_host_user(rows, &n);
	fetch_dmi(rows, &n);

	if (!uname(&un)) {
		frow_push(rows, &n, "OS", "mohos %s", un.sysname);
		frow_push(rows, &n, "Kernel", "%s", un.release);
		frow_push(rows, &n, "Arch", "%s", un.machine);
	}

	fetch_uptime(rows, &n);
	fetch_cpu(rows, &n);
	fetch_gpu(rows, &n);
	fetch_ram(rows, &n);
	fetch_disk(rows, &n);

	fetch_render(rows, n);
	return 0;
}

DEFINE_CMD(mohfetch, "/admin/dev/mohfetch", "system info fetch", do_mohfetch);