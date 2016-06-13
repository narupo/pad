PROG := ../bin/cap
RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c11 -Wno-unused-function -D_DEBUG
CMDS := cap.c
HEADERS := $(CMDS:.c=.h)
OBJS := $(CMDS:.c=.o) 
DEPS := $(CMDS:.c=.d)
BINDIR := ../bin

all: cap cap-cat

cap: cap.c
	$(CC) $(CFLAGS) -o $(PROG) $^ $(LIBS)

cap-cat: cap-cat.c
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(PROG) $(OBJS) $(DEPS)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)

