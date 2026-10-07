#include <stdio.h>

#include "mohos.h"

static int do_cat(struct shell *sh, int argc, char **argv)
{
	struct vfs_node *n;

	if (argc < 2) {
		mohos_pr_err("cat: missing operand\n");
		return -1;
	}

	n = vfs_resolve(sh->cwd, argv[1]);
	if (!n) {
		mohos_pr_err("cat: %s: no such file\n", argv[1]);
		return -1;
	}

	if (n->type == VFS_REG) {
		fputs(n->data ? n->data : "", stdout);
		return 0;
	}

	if (n->type == VFS_CMD) {
		fputs(n->cmd->help, stdout);
		putchar('\n');
		return 0;
	}

	mohos_pr_err("cat: %s: is a directory\n", argv[1]);
	return -1;
}

DEFINE_CMD(cat, "/admin/dev/cat", "print file contents", do_cat);
