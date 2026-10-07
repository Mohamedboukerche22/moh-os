#include <stdio.h>

#include "mohos.h"

static int do_echo(struct shell *sh, int argc, char **argv)
{
	int i;

	(void)sh;

	for (i = 1; i < argc; i++)
		printf("%s%s", argv[i], i + 1 < argc ? " " : "");

	putchar('\n');
	return 0;
}

DEFINE_CMD(echo, "/admin/dev/echo", "print arguments", do_echo);
