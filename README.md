# mohos

A tiny, kernel-flavored command shell with a virtual filesystem — written in clean C.

Type one command and watch a system-info **mohfetch** (logo included):

```sh
$ mohfetch

   .--.          Host: mohamed@archlinux
  |o_o |         OS: mohos Linux
  |:_/ |         Kernel: 7.1.9-arch1-2
 //   \ \        Uptime: 0d 5h 5m
(|     | )       CPU: 11th Gen Intel(R) Core(TM) i3-1115G4 @ 3.00GHz (4)
/'\_   _/`\      GPU: Intel Tiger Lake-LP GT2 [UHD Graphics G4]
\___)=(___/      RAM: 603MiB / 3664MiB
                 Disk: 56GiB / 237GiB (23%)
                 Arch: x86_64
```

## The idea

Everything the shell knows lives in an in-memory filesystem tree:

```
/                 root
├── admin/        admin area
│   ├── dev/      every command is a binary hanging here
│   └── readme.txt
└── user/         user area
    └── notes.txt
```

`cd`, `ls`, `cat`, and even command dispatch all walk the same `struct vfs_node`
tree. Binaries live in `/admin/dev` and are found through a PATH lookup, just
like a real kernel's initramfs.

## Compile & run

Requires `gcc` and `make`.

```sh
make
./mohos
```

Press `Ctrl-D` to exit.

## Architecture

| Path              | What it is                                |
|-------------------|-------------------------------------------|
| `main.c`          | read-eval-print loop                      |
| `shell/shell.c`   | tokenizer + PATH-based command lookup      |
| `fs/vfs.c`        | directory tree, path resolution, `..`/`.` |
| `cmd/`            | one file per command, e.g. `mohfetch`     |
| `include/`        | shared headers                            |

## Add your own command in one file

Commands register themselves through a linker section — no central list to
maintain. Drop a new file in `cmd/` and the Makefile's wildcard picks it up:

```c
#include "mohos.h"

static int do_hello(struct shell *sh, int argc, char **argv)
{
	(void)sh; (void)argc; (void)argv;
	puts("hello from mohos!");
	return 0;
}

DEFINE_CMD(hello, "/admin/dev/hello", "say hello", do_hello);
```

Build, run, and it's already listed under `/admin/dev` in `help`.

```sh
$ help
/admin/dev/hello   say hello
```

## Notes

- As hard to read as real kernel code on purpose — tabs, no comments, `unlikely()`,
  a hand-rolled tokenizer and a chunky dispatch table.