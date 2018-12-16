RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c11 -Wno-unused-function -D_DEBUG
CMDS := \
	cap.c \
	cap-cat.c \
	cap-cd.c \
	cap-home.c \
	cap-pwd.c \
	cap-ls.c \
	cap-alias.c \
	cap-hub.c \
	cap-run.c
MODS := \
	error.c \
	file.c \
	hash.c \
	string.c \
	file.c \
	array.c \
	socket.c \
	util.c \
	env.c \
	cl.c \
	url.c \
    lang-tkr.c \
	var.c
LIBS := -lm -lpthread
BINS := $(CMDS:.c=)
HEADERS := $(CMDS:.c=.h)
CMDOBJS := $(CMDS:.c=.o)
MODOBJS := $(MODS:.c=.o)
DEPS := $(CMDS:.c=.d) $(MODS:.c=.d)
BINDIR := /usr/local/bin/

all: cap tests \
	cap-cat cap-cd cap-home cap-pwd \
	cap-ls cap-alias cap-run cap-hub

cap: cap.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

tests: tests.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-cat: cap-cat.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-cd: cap-cd.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-home: cap-home.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-pwd: cap-pwd.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-ls: cap-ls.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-alias: cap-alias.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-run: cap-run.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

cap-hub: cap-hub.c $(MODOBJS)
	$(CC) $(CFLAGS) -o ./$@ $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(BINS) $(CMDOBJS) $(MODOBJS) $(DEPS)

install: $(BINS)
	install -s $(BINS) $(BINDIR)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)
