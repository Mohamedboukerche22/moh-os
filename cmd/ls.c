#include <stdio.h>

#include "mohos.h"

static const char *ls_sfx(enum vfs_type t)
{
	switch (t) {
	case VFS_DIR:	return "/";
	case VFS_CMD:	return "*";
	default:	return "";
	}
}

static int ls_one(struct vfs_node *n)
{
	return printf("%-20s%s\n", n->name, ls_sfx(n->type)) < 0 ? -1 : 0;
}

static int do_ls(struct shell *sh, int argc, char **argv)
{
	struct vfs_node *n, *c;
	int ret = 0;

	n = argc > 1 ? vfs_resolve(sh->cwd, argv[1]) : sh->cwd;
	if (!n) {
		mohos_pr_err("ls: %s: no such file or directory\n", argv[1]);
		return -1;
	}

	if (n->type != VFS_DIR)
		return ls_one(n);

	for (c = n->child; c; c = c->next)
		ret |= ls_one(c);

	return ret;
}

DEFINE_CMD(ls, "/admin/dev/ls", "list directory entries", do_ls);
