#ifndef _VFS_H
#define _VFS_H

#include <stddef.h>

struct mohos_cmd;

enum vfs_type {
	VFS_DIR,
	VFS_REG,
	VFS_CMD,
};

struct vfs_node {
	char			name[64];
	enum vfs_type		 type;
	struct vfs_node		*parent;
	struct vfs_node		*child;
	struct vfs_node		*next;
	const char		*data;
	const struct mohos_cmd	*cmd;
};

extern struct vfs_node *vfs_root;

extern void vfs_init(void);
extern struct vfs_node *vfs_child(struct vfs_node *dir, const char *name);
extern struct vfs_node *vfs_resolve(struct vfs_node *base, const char *path);
extern struct vfs_node *vfs_mkdir(struct vfs_node *parent, const char *name);
extern struct vfs_node *vfs_mkfile(struct vfs_node *parent, const char *name,
				   const char *data);
extern struct vfs_node *vfs_mkpath(const char *path);
extern struct vfs_node *vfs_mkcmd(const struct mohos_cmd *c);
extern int vfs_path(struct vfs_node *node, char *buf, size_t len);

#endif
