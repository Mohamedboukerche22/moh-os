#include <stdlib.h>
#include <string.h>

#include "mohos.h"

struct vfs_node *vfs_root;

static int vfs_nextcomp(const char **pp, char *buf, size_t len)
{
	const char *p = *pp;
	size_t i = 0;

	while (*p == '/')
		p++;

	if (!*p) {
		*pp = p;
		return 0;
	}

	while (*p && *p != '/') {
		if (i + 1 < len)
			buf[i++] = *p;
		p++;
	}

	buf[i] = '\0';
	*pp = p;
	return 1;
}

static struct vfs_node *vfs_new(struct vfs_node *parent, const char *name,
				enum vfs_type type)
{
	struct vfs_node *n, **slot;

	n = calloc(1, sizeof(*n));
	if (!n)
		return NULL;

	strncpy(n->name, name, sizeof(n->name) - 1);
	n->type = type;
	n->parent = parent;

	if (!parent) {
		vfs_root = n;
		return n;
	}

	for (slot = &parent->child; *slot; slot = &(*slot)->next)
		;

	*slot = n;
	return n;
}

struct vfs_node *vfs_child(struct vfs_node *dir, const char *name)
{
	struct vfs_node *c;

	if (!dir || dir->type != VFS_DIR)
		return NULL;

	for (c = dir->child; c; c = c->next)
		if (!strcmp(c->name, name))
			return c;

	return NULL;
}

struct vfs_node *vfs_resolve(struct vfs_node *base, const char *path)
{
	struct vfs_node *n;
	const char *p = path;
	char comp[NAME_MAX_LEN];

	n = (path[0] == '/') ? vfs_root : (base ? base : vfs_root);

	while (vfs_nextcomp(&p, comp, sizeof(comp))) {
		if (comp[0] == '.' && comp[1] == '\0')
			continue;

		if (comp[0] == '.' && comp[1] == '.' && comp[2] == '\0') {
			if (n->parent)
				n = n->parent;
			continue;
		}

		n = vfs_child(n, comp);
		if (!n)
			return NULL;
	}

	return n;
}

struct vfs_node *vfs_mkdir(struct vfs_node *parent, const char *name)
{
	return vfs_new(parent, name, VFS_DIR);
}

struct vfs_node *vfs_mkfile(struct vfs_node *parent, const char *name,
			    const char *data)
{
	struct vfs_node *n;

	n = vfs_new(parent, name, VFS_REG);
	if (n)
		n->data = data;

	return n;
}

struct vfs_node *vfs_mkpath(const char *path)
{
	struct vfs_node *n = vfs_root;
	const char *p = path;
	char comp[NAME_MAX_LEN];

	while (vfs_nextcomp(&p, comp, sizeof(comp))) {
		struct vfs_node *c = vfs_child(n, comp);

		if (!c)
			c = vfs_mkdir(n, comp);
		if (!c)
			return NULL;
		n = c;
	}

	return n;
}

struct vfs_node *vfs_mkcmd(const struct mohos_cmd *c)
{
	char dir[PATH_MAX_LEN];
	struct vfs_node *parent, *n;
	char *sep;

	strncpy(dir, c->path, sizeof(dir) - 1);
	dir[sizeof(dir) - 1] = '\0';

	sep = strrchr(dir, '/');
	if (!sep) {
		parent = vfs_root;
	} else if (sep == dir) {
		parent = vfs_root;
	} else {
		*sep = '\0';
		parent = vfs_mkpath(dir);
		if (!parent)
			return NULL;
	}

	n = vfs_new(parent, sep ? sep + 1 : dir, VFS_CMD);
	if (n)
		n->cmd = c;

	return n;
}

int vfs_path(struct vfs_node *node, char *buf, size_t len)
{
	const struct vfs_node *stack[32];
	struct vfs_node *n = node;
	size_t used = 0;
	int top = 0;

	if (!n || len < 2)
		return -1;

	for (; n && n->parent; n = n->parent) {
		if (top == (int)ARRAY_SIZE(stack))
			return -1;
		stack[top++] = n;
	}

	buf[used++] = '/';
	buf[used] = '\0';

	while (top) {
		const struct vfs_node *e = stack[--top];
		size_t nl = strlen(e->name);

		if (used > 1) {
			if (used + 1 + nl + 1 > len)
				return -1;
			buf[used++] = '/';
		} else {
			if (used + nl + 1 > len)
				return -1;
		}

		memcpy(buf + used, e->name, nl);
		used += nl;
		buf[used] = '\0';
	}

	return 0;
}

void vfs_init(void)
{
	const struct mohos_cmd *c;
	struct vfs_node *user, *admin, *dev;

	vfs_new(NULL, "/", VFS_DIR);

	user = vfs_mkdir(vfs_root, "user");
	admin = vfs_mkdir(vfs_root, "admin");

	vfs_mkfile(user, "notes.txt",
		   "mohos notes:\n- everything lives under /admin and /user\n"
		   "- binaries are in /admin/dev\n");
	vfs_mkfile(admin, "readme.txt",
		   "admin area, restricted.\n");

	dev = vfs_mkdir(admin, "dev");
	if (!dev)
		return;

	for_each_cmd(c)
		vfs_mkcmd(c);
}
