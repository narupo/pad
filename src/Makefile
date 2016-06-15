PROG := ../bin/cap
RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c11 -Wno-unused-function -D_DEBUG
CMDS := cap.c cap-cat.c
MODS := error.c file.c hash.c string.c config.c file.c
LIBS := -lm
HEADERS := $(CMDS:.c=.h)
CMDOBJS := $(CMDS:.c=.o)
MODOBJS := $(MODS:.c=.o)
DEPS := $(CMDS:.c=.d) $(MODS:.c=.d)
BINDIR := ../bin

all: cap cap-cat

cap: cap.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(PROG) $^ $(LIBS)

cap-cat: cap-cat.c $(MODOBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(PROG) $(CMDOBJS) $(MODOBJS) $(DEPS)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)
