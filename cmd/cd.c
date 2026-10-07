#include "mohos.h"

static int do_cd(struct shell *sh, int argc, char **argv)
{
	struct vfs_node *n;

	n = argc > 1 ? vfs_resolve(sh->cwd, argv[1]) : vfs_root;
	if (!n) {
		mohos_pr_err("cd: %s: no such directory\n", argv[1]);
		return -1;
	}

	if (n->type != VFS_DIR) {
		mohos_pr_err("cd: %s: not a directory\n", argv[1]);
		return -1;
	}

	sh->cwd = n;
	return 0;
}

DEFINE_CMD(cd, "/admin/dev/cd", "change directory", do_cd);
