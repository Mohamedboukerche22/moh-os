#include <stdio.h>

#include "mohos.h"

static int do_pwd(struct shell *sh, int argc, char **argv)
{
	char buf[PATH_MAX_LEN];

	(void)argc;
	(void)argv;

	if (vfs_path(sh->cwd, buf, sizeof(buf)))
		return -1;

	puts(buf);
	return 0;
}

DEFINE_CMD(pwd, "/admin/dev/pwd", "print working directory", do_pwd);
