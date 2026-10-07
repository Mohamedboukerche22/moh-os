#ifndef _CMD_H
#define _CMD_H

struct shell;

struct mohos_cmd {
	const char	*path;
	const char	*help;
	int		(*exec)(struct shell *sh, int argc, char **argv);
};

#define DEFINE_CMD(_sym, _path, _help, _fn)				\
	static const struct mohos_cmd __mohos_cmd_##_sym			\
	__attribute__((section("mohos_cmds"), used, aligned(8))) = {	\
		.path	= (_path),					\
		.help	= (_help),					\
		.exec	= (_fn),					\
	}

extern const struct mohos_cmd __start_mohos_cmds[];
extern const struct mohos_cmd __stop_mohos_cmds[];

#define for_each_cmd(c) \
	for ((c) = __start_mohos_cmds; (c) < __stop_mohos_cmds; (c)++)

#endif
