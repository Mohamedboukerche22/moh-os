#include <stdio.h>

#include "mohos.h"

static int do_uname(struct shell *sh, int argc, char **argv)
{
	(void)sh;
	(void)argc;
	(void)argv;

	puts("mohos 0.1 x86_64 mohos");
	return 0;
}

DEFINE_CMD(uname, "/admin/dev/uname", "print system name", do_uname);
