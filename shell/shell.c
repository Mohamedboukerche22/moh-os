#include <string.h>

#include "mohos.h"

#define ARGV_MAX	16

static const char *const shell_path[] = {
	"/admin/dev",
	"/bin",
};

void shell_init(struct shell *sh)
{
	sh->cwd = vfs_root;
}

static int shell_split(char *line, char **argv, int max)
{
	int argc = 0;
	char *tok;

	for (tok = strtok(line, " \t\r\n"); tok && argc < max - 1;
	     tok = strtok(NULL, " \t\r\n"))
		argv[argc++] = tok;

	argv[argc] = NULL;
	return argc;
}

static struct vfs_node *shell_lookup(struct shell *sh, const char *name)
{
	struct vfs_node *n, *dir;
	unsigned int i;

	if (strchr(name, '/'))
		return vfs_resolve(sh->cwd, name);

	n = vfs_resolve(sh->cwd, name);
	if (n)
		return n;

	for (i = 0; i < ARRAY_SIZE(shell_path); i++) {
		dir = vfs_resolve(vfs_root, shell_path[i]);
		if (!dir)
			continue;

		n = vfs_child(dir, name);
		if (n)
			return n;
	}

	return NULL;
}

void shell_exec(struct shell *sh, char *line)
{
	struct vfs_node *n;
	char *argv[ARGV_MAX];
	int argc;

	argc = shell_split(line, argv, ARGV_MAX);
	if (!argc)
		return;

	n = shell_lookup(sh, argv[0]);
	if (!n || n->type != VFS_CMD) {
		mohos_pr_err("%s: command not found\n", argv[0]);
		return;
	}

	n->cmd->exec(sh, argc, argv);
}
