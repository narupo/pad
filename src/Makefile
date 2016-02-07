CC=gcc
CFLAGS=-Wall -DDEBUG -g -O2 -std=c99 -Wno-unused-function 
CMDS=cap.c cat.c home.c cd.c pwd.c rm.c deploy.c edit.c \
	editor.c help.c ls.c make.c path.c run.c alias.c brief.c \
	server.c

UTILS=caperr.c atcap.c buffer.c cap-file.c \
	config-server.c config-setting.c config.c \
	csvline.c strmap.c file.c strarray.c term.c util.c environ.c string.c memory.c \
	socket.c http-header.c json.c

OBJS=$(CMDS:.c=.o) $(UTILS:.c=.o)

LIBS=-lpthread -lws2_32

TARGET=~/bin/cap
TESTDIR=./test

all: cap

cap: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIBS)

atcap: atcap.c $(UTILS)
	$(CC) -DTEST_ATCAP $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

buffer: buffer.c 
	$(CC) -DTEST_BUFFER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

string: string.c util.c memory.c term.c csvline.c buffer.c
	$(CC) -DTEST_STRING $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

json: json.c util.c memory.c string.c strmap.c strarray.c term.c caperr.c csvline.c
	$(CC) -DTEST_JSON $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

strmap: strmap.c util.c
	$(CC) -DTEST_STRINGMAP $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

file: file.c buffer.c util.c caperr.c term.c
	$(CC) -DDEBUG -DTEST_FILE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

ls: ls.c $(UTILS)
	$(CC) -DDEBUG -DTEST_LS $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

rm: rm.c $(UTILS)
	$(CC) -DDEBUG -DTEST_RM $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

server: server.c $(UTILS)
	$(CC) -DDEBUG -DTEST_SERVER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

socket: socket.c util.c caperr.c memory.c term.c
	$(CC) -DDEBUG -DTEST_SOCKET $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

http-header: http-header.c
	$(CC) -DDEBUG -DTEST_HTTPHEADER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

run: run.c $(UTILS)
	$(CC) -DDEBUG -DTEST_RUN $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

csvline: csvline.c buffer.c caperr.c util.c term.c
	$(CC) -DTEST_CSVLINE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

caperr: caperr.c term.c
	$(CC) -DTEST_CAPERR -DDEBUG $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

cap-file: cap-file.c $(UTILS) 
	$(CC) -DTEST_CAPFILE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

cat: cat.c $(UTILS)
	$(CC) -DTEST_CAT $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

brief: brief.c $(UTILS)
	$(CC) -DTEST_CAT $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

alias: alias.c $(UTILS)
	$(CC) -DTEST_ALIAS $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

make: make.c run.c cat.c $(UTILS)
	$(CC) -DTEST_MAKE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

.PHONY: clean

clean:
	rm $(TARGET) ./*.o ./test/*

.c.o:
	$(CC) $(CFLAGS) -c $<

