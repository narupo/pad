RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c11 -Wno-unused-function -D_DEBUG
CMDS := cap.c cap-cat.c cap-cd.c cap-home.c cap-pwd.c \
	cap-ls.c cap-alias.c cap-run.c
MODS := error.c file.c hash.c string.c file.c array.c \
	socket.c util.c env.c cl.c var.c
LIBS := -lm
BINS := $(CMDS:.c=)
HEADERS := $(CMDS:.c=.h)
CMDOBJS := $(CMDS:.c=.o)
MODOBJS := $(MODS:.c=.o)
DEPS := $(CMDS:.c=.d) $(MODS:.c=.d)
BINDIR := /usr/local/bin/
TESTDIR := ../test

all: cap tests \
	cap-cat cap-cd cap-home cap-pwd \
	cap-ls cap-alias cap-run

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

# A class

string: string.c
	$(CC) -D_TEST_STRING $(CFLAGS) -o $(TESTDIR)/string $^ $(LIBS)

cl: cl.c
	$(CC) -D_TEST_CL $(CFLAGS) -o $(TESTDIR)/cl $^ $(LIBS)

# B class

util: util.c file.c error.c env.c string.c cl.c
	$(CC) -D_TEST_UTIL $(CFLAGS) -o $(TESTDIR)/util $^ $(LIBS)

var: var.c file.c error.c env.c
	$(CC) -D_TEST_VAR $(CFLAGS) -o $(TESTDIR)/var $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(BINS) $(CMDOBJS) $(MODOBJS) $(DEPS)

install: $(BINS)
	install -s $(BINS) $(BINDIR)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)
