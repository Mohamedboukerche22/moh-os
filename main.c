#include <stdio.h>

#include "mohos.h"


#define CMDBUF_SZ	128

static char cmdbuf[CMDBUF_SZ];

int main(void)
{
	struct shell sh;

	vfs_init();
	shell_init(&sh);

	for (;;) {
		fputs(PROMPT, stdout);
		fflush(stdout);

		if (!fgets(cmdbuf, CMDBUF_SZ, stdin)) {
			putchar('\n');
			break;
		}

		shell_exec(&sh, cmdbuf);
	}

	return 0;
}
