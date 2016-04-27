RM := rm
CC := gcc
CFLAGS := -Wall -g -O0 -std=c99 -Wno-unused-function -D_DEBUG
CMDS := cap.c cat.c home.c cd.c pwd.c trash.c deploy.c edit.c \
	editor.c help.c ls.c make.c path.c run.c alias.c brief.c \
	server.c mkdir.c hub.c

UTILS := caperr.c atcap.c buffer.c cap-file.c config.c \
	csvline.c strmap.c file.c strarray.c term.c util.c environ.c string.c memory.c \
	socket.c http-header.c json.c io.c hash.c shell.c

HEADERS := $(CMDS:.c=.h) $(UTILS:.c=.h) hash.h
OBJS := $(CMDS:.c=.o) $(UTILS:.c=.o)
DEPS := $(CMDS:.c=.d) $(UTILS:.c=.d)

ifeq ($(OS),Windows_NT)
	LIBS := -lpthread -lcrypt -lws2_32
else
	LIBS := -lpthread -lcrypt
endif

PROGRAM := ~/bin/cap
TESTDIR := ./test

all: cap

cap: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $^ $(LIBS)

atcap: atcap.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_ATCAP $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

buffer: buffer.c
	$(CC) -D_DEBUG -D_TEST_BUFFER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

string: string.c memory.c caperr.c csvline.c util.c term.c
	$(CC) -D_DEBUG -D_TEST_STRING $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

json: json.c util.c memory.c string.c strmap.c strarray.c term.c caperr.c csvline.c file.c environ.c hash.c
	$(CC) -D_DEBUG -D_TEST_JSON $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

strmap: strmap.c util.c
	$(CC) -D_DEBUG -D_TEST_STRINGMAP $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

file: file.c buffer.c util.c caperr.c term.c
	$(CC) -D_DEBUG -D_TEST_FILE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

ls: ls.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_LS $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

trash: trash.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_TRASH $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

mkdir: mkdir.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_MKDIR $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

config: config.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_CONFIG $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

server: server.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_SERVER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

socket: socket.c util.c caperr.c memory.c term.c
	$(CC) -D_DEBUG -D_TEST_SOCKET $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

signal: signal.c
	$(CC) -D_DEBUG -D_TEST_SIGNAL $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

http-header: http-header.c
	$(CC) -D_DEBUG -D_TEST_HTTPHEADER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

csvline: csvline.c buffer.c caperr.c util.c term.c
	$(CC) -D_DEBUG -D_TEST_CSVLINE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

caperr: caperr.c term.c
	$(CC) -D_DEBUG -D_TEST_CAPERR $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

cap-file: cap-file.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_CAPFILE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

term: term.c
	$(CC) -D_DEBUG -D_TEST_TERM $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

run: run.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_RUN $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

cat: cat.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_CAT $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

brief: brief.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_CAT $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

alias: alias.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_ALIAS $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

make: make.c run.c cat.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_MAKE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

hub: hub.c $(UTILS)
	$(CC) -D_DEBUG -D_TEST_HUB $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

.PHONY: clean depend

clean:
	$(RM) $(PROGRAM) $(OBJS) $(DEPS)

.c.o:
	$(CC) $(CFLAGS) -c -MMD -MP $<

-include $(DEPS)
