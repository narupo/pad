CC=clang
CFLAGS=-Wall -g -O0 -std=c99
CMDS=cap.c cat.c cd.c deploy.c edit.c \
	editor.c help.c ls.c make.c path.c run.c alias.c brief.c

UTILS=caperr.c atcap.c buffer.c cap-file.c config-setting.c config.c \
	csvline.c file.c strarray.c term.c util.c

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

csvline: csvline.c buffer.c
	$(CC) -DTEST_CSVLINE $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

caperr: caperr.c term.c
	$(CC) -DTEST_CAPERR $(CFLAGS) -o $(TESTDIR)/$@ $^ $(LIBS)

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
