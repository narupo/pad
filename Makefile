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
	-Wno-unused-result \
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
	$(MKDIR) build$(SEP)core
	$(MKDIR) build$(SEP)home
	$(MKDIR) build$(SEP)cd
	$(MKDIR) build$(SEP)pwd
	$(MKDIR) build$(SEP)ls
	$(MKDIR) build$(SEP)cat
	$(MKDIR) build$(SEP)run
	$(MKDIR) build$(SEP)alias
	$(MKDIR) build$(SEP)edit
	$(MKDIR) build$(SEP)editor
	$(MKDIR) build$(SEP)mkdir
	$(MKDIR) build$(SEP)rm
	$(MKDIR) build$(SEP)mv
	$(MKDIR) build$(SEP)cp
	$(MKDIR) build$(SEP)touch
	$(MKDIR) build$(SEP)snippet
	$(MKDIR) build$(SEP)link
	$(MKDIR) build$(SEP)hub
	$(MKDIR) build$(SEP)hub$(SEP)commands
	$(MKDIR) build$(SEP)lang$(SEP)

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
	build/core/config.o \
	build/core/util.o \
	build/core/alias_manager.o \
	build/core/symlink.o \
	build/core/args.o \
	build/home/home.o \
	build/cd/cd.o \
	build/pwd/pwd.o \
	build/ls/ls.o \
	build/cat/cat.o \
	build/run/run.o \
	build/alias/alias.o \
	build/edit/edit.o \
	build/editor/editor.o \
	build/mkdir/mkdir.o \
	build/rm/rm.o \
	build/mv/mv.o \
	build/cp/cp.o \
	build/touch/touch.o \
	build/snippet/snippet.o \
	build/link/link.o \
	build/hub/hub.o \
	build/lang/tokens.o \
	build/lang/tokenizer.o \
	build/lang/nodes.o \
	build/lang/context.o \
	build/lang/ast.o
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
		build/core/config.o \
		build/core/util.o \
		build/core/alias_manager.o \
		build/core/symlink.o \
		build/core/args.o \
		build/home/home.o \
		build/cd/cd.o \
		build/pwd/pwd.o \
		build/ls/ls.o \
		build/cat/cat.o \
		build/run/run.o \
		build/alias/alias.o \
		build/edit/edit.o \
		build/editor/editor.o \
		build/mkdir/mkdir.o \
		build/rm/rm.o \
		build/mv/mv.o \
		build/cp/cp.o \
		build/touch/touch.o \
		build/snippet/snippet.o \
		build/link/link.o \
		build/hub/hub.o \
		build/lang/tokens.o \
		build/lang/tokenizer.o \
		build/lang/nodes.o \
		build/lang/context.o \
		build/lang/ast.o

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
	build/core/config.o \
	build/core/util.o \
	build/core/alias_manager.o \
	build/core/symlink.o \
	build/core/args.o \
	build/home/home.o \
	build/cd/cd.o \
	build/pwd/pwd.o \
	build/ls/ls.o \
	build/cat/cat.o \
	build/run/run.o \
	build/alias/alias.o \
	build/edit/edit.o \
	build/editor/editor.o \
	build/mkdir/mkdir.o \
	build/rm/rm.o \
	build/mv/mv.o \
	build/cp/cp.o \
	build/touch/touch.o \
	build/snippet/snippet.o \
	build/link/link.o \
	build/hub/hub.o \
	build/lang/tokens.o \
	build/lang/tokenizer.o \
	build/lang/nodes.o \
	build/lang/context.o \
	build/lang/ast.o
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
		build/core/config.o \
		build/core/util.o \
		build/core/alias_manager.o \
		build/core/symlink.o \
		build/core/args.o \
		build/home/home.o \
		build/cd/cd.o \
		build/pwd/pwd.o \
		build/ls/ls.o \
		build/cat/cat.o \
		build/run/run.o \
		build/alias/alias.o \
		build/edit/edit.o \
		build/editor/editor.o \
		build/mkdir/mkdir.o \
		build/rm/rm.o \
		build/mv/mv.o \
		build/cp/cp.o \
		build/touch/touch.o \
		build/snippet/snippet.o \
		build/link/link.o \
		build/hub/hub.o \
		build/lang/tokens.o \
		build/lang/tokenizer.o \
		build/lang/nodes.o \
		build/lang/context.o \
		build/lang/ast.o

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
	
build/core/config.o: src/core/config.c src/core/config.h
	$(CC) $(CFLAGS) -c $< -o $@

build/core/util.o: src/core/util.c src/core/util.h
	$(CC) $(CFLAGS) -c $< -o $@

build/core/alias_manager.o: src/core/alias_manager.c src/core/alias_manager.h
	$(CC) $(CFLAGS) -c $< -o $@

build/core/symlink.o: src/core/symlink.c src/core/symlink.h
	$(CC) $(CFLAGS) -c $< -o $@

build/core/args.o: src/core/args.c src/core/args.h
	$(CC) $(CFLAGS) -c $< -o $@

build/home/home.o: src/home/home.c src/home/home.h
	$(CC) $(CFLAGS) -c $< -o $@

build/cd/cd.o: src/cd/cd.c src/cd/cd.h
	$(CC) $(CFLAGS) -c $< -o $@

build/pwd/pwd.o: src/pwd/pwd.c src/pwd/pwd.h
	$(CC) $(CFLAGS) -c $< -o $@

build/ls/ls.o: src/ls/ls.c src/ls/ls.h
	$(CC) $(CFLAGS) -c $< -o $@

build/cat/cat.o: src/cat/cat.c src/cat/cat.h
	$(CC) $(CFLAGS) -c $< -o $@

build/run/run.o: src/run/run.c src/run/run.h
	$(CC) $(CFLAGS) -c $< -o $@

build/alias/alias.o: src/alias/alias.c src/alias/alias.h
	$(CC) $(CFLAGS) -c $< -o $@

build/edit/edit.o: src/edit/edit.c src/edit/edit.h
	$(CC) $(CFLAGS) -c $< -o $@

build/editor/editor.o: src/editor/editor.c src/editor/editor.h
	$(CC) $(CFLAGS) -c $< -o $@

build/mkdir/mkdir.o: src/mkdir/mkdir.c src/mkdir/mkdir.h
	$(CC) $(CFLAGS) -c $< -o $@

build/rm/rm.o: src/rm/rm.c src/rm/rm.h
	$(CC) $(CFLAGS) -c $< -o $@

build/mv/mv.o: src/mv/mv.c src/mv/mv.h
	$(CC) $(CFLAGS) -c $< -o $@

build/cp/cp.o: src/cp/cp.c src/cp/cp.h
	$(CC) $(CFLAGS) -c $< -o $@

build/touch/touch.o: src/touch/touch.c src/touch/touch.h
	$(CC) $(CFLAGS) -c $< -o $@

build/snippet/snippet.o: src/snippet/snippet.c src/snippet/snippet.h
	$(CC) $(CFLAGS) -c $< -o $@

build/link/link.o: src/link/link.c src/link/link.h
	$(CC) $(CFLAGS) -c $< -o $@

build/hub/hub.o: src/hub/hub.c src/hub/hub.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lang/tokenizer.o: src/lang/tokenizer.c src/lang/tokenizer.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lang/tokens.o: src/lang/tokens.c src/lang/tokens.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lang/nodes.o: src/lang/nodes.c src/lang/nodes.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lang/ast.o: src/lang/ast.c src/lang/ast.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lang/context.o: src/lang/context.c src/lang/context.h
	$(CC) $(CFLAGS) -c $< -o $@

