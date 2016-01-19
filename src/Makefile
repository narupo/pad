CC=gcc
CFLAGS=-Wall -g -O2 -std=c99 -Wno-unused-function 
CMDS=cap.c cat.c root.c cd.c deploy.c edit.c \
	editor.c help.c ls.c make.c path.c run.c alias.c brief.c

UTILS=caperr.c atcap.c buffer.c cap-file.c config-setting.c config.c \
	csvline.c file.c strarray.c term.c util.c environ.c

LIBS=-lpthread

TARGET=~/bin/cap
TESTDIR=./test

all: cap

cap: $(CMDS) $(UTILS)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIBS)

atcap: atcap.c $(UTILS)
	$(CC) -DTEST_ATCAP $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

buffer: buffer.c 
	$(CC) -DTEST_BUFFER $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

file: file.c buffer.c util.c caperr.c term.c
	$(CC) -DDEBUG -DTEST_FILE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

ls: ls.c $(UTILS)
	$(CC) -DDEBUG -DTEST_LS $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

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
	rm ./cap ./test/*

.c.o:
	$(CC) $(CFLAGS) -c $<

