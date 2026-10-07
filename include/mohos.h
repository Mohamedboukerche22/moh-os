#ifndef _MOHOS_H
#define _MOHOS_H

#include <stddef.h>
#include <stdio.h>

#include "vfs.h"
#include "cmd.h"
#include "shell.h"

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define unlikely(x)	__builtin_expect(!!(x), 0)

#define mohos_pr_err(fmt, ...) \
	fprintf(stderr, "mohos: " fmt, ##__VA_ARGS__)

#define PROMPT		"$ "
#define PATH_MAX_LEN	256
#define NAME_MAX_LEN	64

#endif
