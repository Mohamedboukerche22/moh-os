#include <stdio.h>

#include "mohos.h"

static int do_help(struct shell *sh, int argc, char **argv)
{
	const struct mohos_cmd *c;

	(void)sh;
	(void)argc;
	(void)argv;

	for_each_cmd(c)
		printf("%-24s %s\n", c->path, c->help);

	return 0;
}

DEFINE_CMD(help, "/admin/dev/help", "show this help", do_help);
