ifeq ($(OS), Windows_NT)
	RM := del
	RMDIR := rmdir /s /q
	SEP := \\
else
	RM := rm
	RMDIR := rm -rf
	SEP := /
endif
MKDIR := mkdir
CC := gcc
INCLUDE := src
CFLAGS := -Wall \
	-g \
	-O3 \
	-std=c11 \
	-Wno-unused-function \
	-D_DEBUG \
	-I$(INCLUDE) \

# $(warning $(wildcard src/*.c))

all: tests cap

.PHONY: clean
clean:
	$(RMDIR) build

.PHONY: init
init:
	$(MKDIR) build
	$(MKDIR) build$(SEP)lib
	$(MKDIR) build$(SEP)modules
	$(MKDIR) build$(SEP)modules$(SEP)commands
	$(MKDIR) build$(SEP)modules$(SEP)lang

.PHONY: cc
cc:
	$(CC) -v

tests: build/tests.o \
	build/lib/error.o \
	build/lib/memory.o \
	build/lib/file.o \
	build/lib/cstring.o \
	build/lib/string.o \
	build/lib/cstring_array.o \
	build/lib/cl.o \
	build/lib/format.o \
	build/lib/dict.o \
	build/modules/config.o \
	build/modules/util.o \
	build/modules/alias_manager.o \
	build/modules/symlink.o \
	build/modules/commands/home.o \
	build/modules/commands/cd.o \
	build/modules/commands/pwd.o \
	build/modules/commands/ls.o \
	build/modules/commands/cat.o \
	build/modules/commands/run.o \
	build/modules/commands/alias.o \
	build/modules/commands/edit.o \
	build/modules/commands/editor.o \
	build/modules/commands/mkdir.o \
	build/modules/commands/rm.o \
	build/modules/commands/mv.o \
	build/modules/commands/cp.o \
	build/modules/commands/touch.o \
	build/modules/commands/snippet.o \
	build/modules/commands/link.o \
	build/modules/lang/tokens.o \
	build/modules/lang/tokenizer.o \
	build/modules/lang/nodes.o \
	build/modules/lang/context.o \
	build/modules/lang/ast.o
	$(CC) $(CFLAGS) -o build/tests \
		build/tests.o \
		build/lib/error.o \
		build/lib/memory.o \
		build/lib/file.o \
		build/lib/cstring.o \
		build/lib/string.o \
		build/lib/cstring_array.o \
		build/lib/cl.o \
		build/lib/format.o \
		build/lib/dict.o \
		build/modules/config.o \
		build/modules/util.o \
		build/modules/alias_manager.o \
		build/modules/symlink.o \
		build/modules/commands/home.o \
		build/modules/commands/cd.o \
		build/modules/commands/pwd.o \
		build/modules/commands/ls.o \
		build/modules/commands/cat.o \
		build/modules/commands/run.o \
		build/modules/commands/alias.o \
		build/modules/commands/edit.o \
		build/modules/commands/editor.o \
		build/modules/commands/mkdir.o \
		build/modules/commands/rm.o \
		build/modules/commands/mv.o \
		build/modules/commands/cp.o \
		build/modules/commands/touch.o \
		build/modules/commands/snippet.o \
		build/modules/commands/link.o \
		build/modules/lang/tokens.o \
		build/modules/lang/tokenizer.o \
		build/modules/lang/nodes.o \
		build/modules/lang/context.o \
		build/modules/lang/ast.o

build/tests.o: src/tests.c src/tests.h
	$(CC) $(CFLAGS) -c $< -o $@

cap: build/app.o \
	build/lib/error.o \
	build/lib/memory.o \
	build/lib/file.o \
	build/lib/cstring.o \
	build/lib/string.o \
	build/lib/cstring_array.o \
	build/lib/cl.o \
	build/lib/format.o \
	build/lib/dict.o \
	build/modules/config.o \
	build/modules/util.o \
	build/modules/alias_manager.o \
	build/modules/symlink.o \
	build/modules/commands/home.o \
	build/modules/commands/cd.o \
	build/modules/commands/pwd.o \
	build/modules/commands/ls.o \
	build/modules/commands/cat.o \
	build/modules/commands/run.o \
	build/modules/commands/alias.o \
	build/modules/commands/edit.o \
	build/modules/commands/editor.o \
	build/modules/commands/mkdir.o \
	build/modules/commands/rm.o \
	build/modules/commands/mv.o \
	build/modules/commands/cp.o \
	build/modules/commands/touch.o \
	build/modules/commands/snippet.o \
	build/modules/commands/link.o \
	build/modules/lang/tokens.o \
	build/modules/lang/tokenizer.o \
	build/modules/lang/nodes.o \
	build/modules/lang/context.o \
	build/modules/lang/ast.o
	$(CC) $(CFLAGS) -o build/cap \
		build/app.o \
		build/lib/error.o \
		build/lib/memory.o \
		build/lib/file.o \
		build/lib/cstring.o \
		build/lib/string.o \
		build/lib/cstring_array.o \
		build/lib/cl.o \
		build/lib/format.o \
		build/lib/dict.o \
		build/modules/config.o \
		build/modules/util.o \
		build/modules/alias_manager.o \
		build/modules/symlink.o \
		build/modules/commands/home.o \
		build/modules/commands/cd.o \
		build/modules/commands/pwd.o \
		build/modules/commands/ls.o \
		build/modules/commands/cat.o \
		build/modules/commands/run.o \
		build/modules/commands/alias.o \
		build/modules/commands/edit.o \
		build/modules/commands/editor.o \
		build/modules/commands/mkdir.o \
		build/modules/commands/rm.o \
		build/modules/commands/mv.o \
		build/modules/commands/cp.o \
		build/modules/commands/touch.o \
		build/modules/commands/snippet.o \
		build/modules/commands/link.o \
		build/modules/lang/tokens.o \
		build/modules/lang/tokenizer.o \
		build/modules/lang/nodes.o \
		build/modules/lang/context.o \
		build/modules/lang/ast.o

build/app.o: src/app.c src/app.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/error.o: src/lib/error.c src/lib/error.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/memory.o: src/lib/memory.c src/lib/memory.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/file.o: src/lib/file.c src/lib/file.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/cstring.o: src/lib/cstring.c src/lib/cstring.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/string.o: src/lib/string.c src/lib/string.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/cstring_array.o: src/lib/cstring_array.c src/lib/cstring_array.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/cl.o: src/lib/cl.c src/lib/cl.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/format.o: src/lib/format.c src/lib/format.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/dict.o: src/lib/dict.c src/lib/dict.h
	$(CC) $(CFLAGS) -c $< -o $@
	
build/modules/config.o: src/modules/config.c src/modules/config.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/util.o: src/modules/util.c src/modules/util.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/alias_manager.o: src/modules/alias_manager.c src/modules/alias_manager.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/symlink.o: src/modules/symlink.c src/modules/symlink.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/home.o: src/modules/commands/home.c src/modules/commands/home.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/cd.o: src/modules/commands/cd.c src/modules/commands/cd.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/pwd.o: src/modules/commands/pwd.c src/modules/commands/pwd.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/ls.o: src/modules/commands/ls.c src/modules/commands/ls.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/cat.o: src/modules/commands/cat.c src/modules/commands/cat.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/run.o: src/modules/commands/run.c src/modules/commands/run.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/alias.o: src/modules/commands/alias.c src/modules/commands/alias.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/edit.o: src/modules/commands/edit.c src/modules/commands/edit.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/editor.o: src/modules/commands/editor.c src/modules/commands/editor.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/mkdir.o: src/modules/commands/mkdir.c src/modules/commands/mkdir.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/rm.o: src/modules/commands/rm.c src/modules/commands/rm.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/mv.o: src/modules/commands/mv.c src/modules/commands/mv.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/cp.o: src/modules/commands/cp.c src/modules/commands/cp.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/touch.o: src/modules/commands/touch.c src/modules/commands/touch.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/snippet.o: src/modules/commands/snippet.c src/modules/commands/snippet.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/commands/link.o: src/modules/commands/link.c src/modules/commands/link.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/lang/tokenizer.o: src/modules/lang/tokenizer.c src/modules/lang/tokenizer.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/lang/tokens.o: src/modules/lang/tokens.c src/modules/lang/tokens.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/lang/nodes.o: src/modules/lang/nodes.c src/modules/lang/nodes.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/lang/ast.o: src/modules/lang/ast.c src/modules/lang/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules/lang/context.o: src/modules/lang/context.c src/modules/lang/context.h
	$(CC) $(CFLAGS) -c $< -o $@

