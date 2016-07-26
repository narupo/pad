RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c11 -Wno-unused-function -D_DEBUG
CMDS := cap.c cap-cat.c cap-cd.c cap-home.c cap-pwd.c \
	cap-ls.c cap-alias.c cap-run.c
MODS := error.c file.c hash.c string.c config.c file.c map.c array.c \
	socket.c util.c
LIBS := -lm
HEADERS := $(CMDS:.c=.h)
CMDOBJS := $(CMDS:.c=.o)
MODOBJS := $(MODS:.c=.o)
DEPS := $(CMDS:.c=.d) $(MODS:.c=.d)
BINDIR := ../bin

all: cap cap-cat cap-cd cap-home cap-pwd \
	cap-ls cap-alias cap-run

cap: cap.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-cat: cap-cat.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-cd: cap-cd.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-home: cap-home.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-pwd: cap-pwd.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-ls: cap-ls.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-alias: cap-alias.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

cap-run: cap-run.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(BINDIR)/cap $(BINDIR)/cap-* $(CMDOBJS) $(MODOBJS) $(DEPS)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)
