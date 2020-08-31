# get rm and rmdir and sep
ifeq ($(OS), Windows_NT)
	RM := del
	RMDIR := rmdir /s /q
	SEP := \\
else
	RM := rm
	RMDIR := rm -rf
	SEP := /
endif

# windows's mkdir not has -p option :/
MKDIR := mkdir
CC := gcc
INCLUDE := src

ifeq ($(OS), Windows_NT)
	CFLAGS := -Wall \
		-g \
		-O0 \
		-std=c11 \
		-Wno-unused-function \
		-Wno-unused-result \
		-D_DEBUG \
		-I$(INCLUDE) \
		-LD:\\lib \
		-lws2_32
else
	CFLAGS := -Wall \
		-g \
		-O0 \
		-std=c11 \
		-Wno-unused-function \
		-Wno-unused-result \
		-D_DEBUG \
		-I$(INCLUDE)
endif

# this is benri tool
# $(warning $(wildcard src/*.c))

all: tests cap

.PHONY: clean
clean:
	$(RMDIR) build

.PHONY: init
init:
	$(MKDIR) \
	build \
	build$(SEP)lib \
	build$(SEP)core \
	build$(SEP)home \
	build$(SEP)cd \
	build$(SEP)pwd \
	build$(SEP)ls \
	build$(SEP)cat \
	build$(SEP)run \
	build$(SEP)exec \
	build$(SEP)alias \
	build$(SEP)edit \
	build$(SEP)editor \
	build$(SEP)mkdir \
	build$(SEP)rm \
	build$(SEP)mv \
	build$(SEP)cp \
	build$(SEP)touch \
	build$(SEP)snippet \
	build$(SEP)link \
	build$(SEP)hub \
	build$(SEP)hub$(SEP)commands \
	build$(SEP)make \
	build$(SEP)sh \
	build$(SEP)find \
	build$(SEP)lang$(SEP) \
	build$(SEP)lang$(SEP)builtin \
	build$(SEP)lang$(SEP)builtin$(SEP)modules

.PHONY: cc
cc:
	$(CC) -v

SRCS := build/lib/error.c \
	build/lib/memory.c \
	build/lib/file.c \
	build/lib/cstring.c \
	build/lib/string.c \
	build/lib/unicode.c \
	build/lib/cstring_array.c \
	build/lib/cl.c \
	build/lib/format.c \
	build/lib/dict.c \
	build/lib/cmdline.c \
	build/lib/pipe.c \
	build/lib/term.c \
	build/lib/path.c \
	build/lib/socket.c \
	build/core/config.c \
	build/core/util.c \
	build/core/alias_manager.c \
	build/core/alias_info.c \
	build/core/symlink.c \
	build/core/args.c \
	build/core/error_stack.c \
	build/home/home.c \
	build/cd/cd.c \
	build/pwd/pwd.c \
	build/ls/ls.c \
	build/cat/cat.c \
	build/run/run.c \
	build/exec/exec.c \
	build/alias/alias.c \
	build/edit/edit.c \
	build/editor/editor.c \
	build/mkdir/mkdir.c \
	build/rm/rm.c \
	build/mv/mv.c \
	build/cp/cp.c \
	build/touch/touch.c \
	build/snippet/snippet.c \
	build/link/link.c \
	build/hub/hub.c \
	build/make/make.c \
	build/sh/sh.c \
	build/find/find.c \
	build/find/arguments_manager.c \
	build/lang/tokens.c \
	build/lang/tokenizer.c \
	build/lang/nodes.c \
	build/lang/context.c \
	build/lang/ast.c \
	build/lang/compiler.c \
	build/lang/traverser.c \
	build/lang/object.c \
	build/lang/object_array.c \
	build/lang/object_dict.c \
	build/lang/node_array.c \
	build/lang/node_dict.c \
	build/lang/opts.c \
	build/lang/scope.c \
	build/lang/utils.c \
	build/lang/gc.c \
	build/lang/kit.c \
	build/lang/importer.c \
	build/lang/arguments.c \
	build/lang/chain_node.c \
	build/lang/chain_nodes.c \
	build/lang/chain_object.c \
	build/lang/chain_objects.c \
	build/lang/builtin/functions.c \
	build/lang/builtin/modules/unicode.c \
	build/lang/builtin/modules/array.c \
	build/lang/builtin/modules/dict.c \
	build/lang/builtin/modules/opts.c \
	build/lang/builtin/modules/alias.c \

OBJS := $(SRCS:.c=.o)

tests: build/tests.o $(OBJS)
	$(CC) $(CFLAGS) -o build/tests $^

cap: build/app.o $(OBJS)
	$(CC) $(CFLAGS) -o build/cap $^

build/app.o: src/app.c src/app.h
	$(CC) $(CFLAGS) -c $< -o $@
build/tests.o: src/tests.c src/tests.h
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
build/lib/unicode.o: src/lib/unicode.c src/lib/unicode.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cstring_array.o: src/lib/cstring_array.c src/lib/cstring_array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cl.o: src/lib/cl.c src/lib/cl.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/format.o: src/lib/format.c src/lib/format.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/dict.o: src/lib/dict.c src/lib/dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cmdline.o: src/lib/cmdline.c src/lib/cmdline.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/pipe.o: src/lib/pipe.c src/lib/pipe.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/term.o: src/lib/term.c src/lib/term.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/path.o: src/lib/path.c src/lib/path.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/socket.o: src/lib/socket.c src/lib/socket.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/config.o: src/core/config.c src/core/config.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/util.o: src/core/util.c src/core/util.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_manager.o: src/core/alias_manager.c src/core/alias_manager.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_info.o: src/core/alias_info.c src/core/alias_info.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/symlink.o: src/core/symlink.c src/core/symlink.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/error_stack.o: src/core/error_stack.c src/core/error_stack.h
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
build/exec/exec.o: src/exec/exec.c src/exec/exec.h
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
build/make/make.o: src/make/make.c src/make/make.h
	$(CC) $(CFLAGS) -c $< -o $@
build/sh/sh.o: src/sh/sh.c src/sh/sh.h
	$(CC) $(CFLAGS) -c $< -o $@
build/find/find.o: src/find/find.c src/find/find.h
	$(CC) $(CFLAGS) -c $< -o $@
build/find/arguments_manager.o: src/find/arguments_manager.c src/find/arguments_manager.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/tokenizer.o: src/lang/tokenizer.c src/lang/tokenizer.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/tokens.o: src/lang/tokens.c src/lang/tokens.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/nodes.o: src/lang/nodes.c src/lang/nodes.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/ast.o: src/lang/ast.c src/lang/ast.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/compiler.o: src/lang/compiler.c src/lang/compiler.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/traverser.o: src/lang/traverser.c src/lang/traverser.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/context.o: src/lang/context.c src/lang/context.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/object.o: src/lang/object.c src/lang/object.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/object_array.o: src/lang/object_array.c src/lang/object_array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/object_dict.o: src/lang/object_dict.c src/lang/object_dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/node_array.o: src/lang/node_array.c src/lang/node_array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/node_dict.o: src/lang/node_dict.c src/lang/node_dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/opts.o: src/lang/opts.c src/lang/opts.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/scope.o: src/lang/scope.c src/lang/scope.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/utils.o: src/lang/utils.c src/lang/utils.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/gc.o: src/lang/gc.c src/lang/gc.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/kit.o: src/lang/kit.c src/lang/kit.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/importer.o: src/lang/importer.c src/lang/importer.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/arguments.o: src/lang/arguments.c src/lang/arguments.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_node.o: src/lang/chain_node.c src/lang/chain_node.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_nodes.o: src/lang/chain_nodes.c src/lang/chain_nodes.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_object.o: src/lang/chain_object.c src/lang/chain_object.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_objects.o: src/lang/chain_objects.c src/lang/chain_objects.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/functions.o: src/lang/builtin/functions.c src/lang/builtin/functions.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/unicode.o: src/lang/builtin/modules/unicode.c src/lang/builtin/modules/unicode.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/array.o: src/lang/builtin/modules/array.c src/lang/builtin/modules/array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/dict.o: src/lang/builtin/modules/dict.c src/lang/builtin/modules/dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/opts.o: src/lang/builtin/modules/opts.c src/lang/builtin/modules/opts.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/alias.o: src/lang/builtin/modules/alias.c src/lang/builtin/modules/alias.h
	$(CC) $(CFLAGS) -c $< -o $@

