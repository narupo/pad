PROG := ../bin/cap
RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c11 -Wno-unused-function -D_DEBUG
CMDS := cap.c
HEADERS := $(CMDS:.c=.h)
OBJS := $(CMDS:.c=.o) 
DEPS := $(CMDS:.c=.d)
TESTDIR := ./test

all: cap

cap: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(PROG) $(OBJS) $(DEPS)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)

