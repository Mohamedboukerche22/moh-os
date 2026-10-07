CC	:= gcc
CFLAGS	:= -std=gnu11 -O2 -Wall -Wextra -Iinclude -MMD -MP
LDLIBS	:=

SRCS	:= main.c fs/vfs.c shell/shell.c $(wildcard cmd/*.c)
OBJS	:= $(SRCS:.c=.o)
DEPS	:= $(OBJS:.o=.d)
TARGET	:= mohos

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

.PHONY: all clean
