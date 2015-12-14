CC=clang
CFLAGS=-Wall -g -O0 -std=c99
CMDS=cap.c cat.c cd.c deploy.c edit.c \
	editor.c help.c ls.c make.c path.c run.c

UTILS=atcap.c buffer.c cap-file.c config-setting.c config.c \
	csvline.c file.c strarray.c term.c util.c

TARGET=cap
TESTDIR=./test

all: cap

cap: $(CMDS) $(UTILS)
	$(CC) $(CFLAGS) -o $@ $^

atcap: atcap.c $(UTILS)
	$(CC) -DTEST_ATCAP $(CFLAGS) -o $(TESTDIR)/$@ $^

buffer: buffer.c 
	$(CC) -DTEST_BUFFER $(CFLAGS) -o $(TESTDIR)/$@ $^

cap-file: cap-file.c $(UTILS) 
	$(CC) -DTEST_CAPFILE $(CFLAGS) -o $(TESTDIR)/$@ $^

cat: cat.c $(UTILS)
	$(CC) -DTEST_CAT $(CFLAGS) -o $(TESTDIR)/$@ $^

make: make.c run.c cat.c $(UTILS)
	$(CC) -DTEST_MAKE $(CFLAGS) -o $(TESTDIR)/$@ $^

.PHONY: clean

clean:
	rm ./cap ./test/*

.c.o:
	$(CC) $(CFLAGS) -c $<
