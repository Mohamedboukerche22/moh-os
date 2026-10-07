#ifndef _SHELL_H
#define _SHELL_H

#include "vfs.h"

struct shell {
	struct vfs_node *cwd;
};

extern void shell_init(struct shell *sh);
extern void shell_exec(struct shell *sh, char *line);

#endif
