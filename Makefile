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
INCLUDE := .

ifeq ($(OS), Windows_NT)
	CFLAGS := -Wall \
		-g \
		-O0 \
		-std=c11 \
		-Wno-unused-function \
		-Wno-unused-result \
		-D_DEBUG \
		-I$(INCLUDE)
	OUTLIB := libpad.dll
else
	CFLAGS := -Wall \
		-g \
		-O0 \
		-std=c11 \
		-Wno-unused-function \
		-Wno-unused-result \
		-D_DEBUG \
		-I$(INCLUDE) \
		-fPIC
	OUTLIB := libpad.so
endif

# this is benri tool
# $(warning $(wildcard pad/*.c))

all: tests pad lib

.PHONY: clean
clean:
	$(RMDIR) build

.PHONY: init
init:
	$(MKDIR) \
	build \
	build$(SEP)tests \
	build$(SEP)lib \
	build$(SEP)core \
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
	build/lib/void_dict.c \
	build/lib/cmdline.c \
	build/lib/pipe.c \
	build/lib/term.c \
	build/lib/path.c \
	build/lib/socket.c \
	build/core/config.c \
	build/core/util.c \
	build/core/alias_info.c \
	build/core/args.c \
	build/core/error_stack.c \
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
	build/lang/builtin/structs.c \
	build/lang/builtin/functions.c \
	build/lang/builtin/func_info_array.c \
	build/lang/builtin/module.c \
	build/lang/builtin/modules/unicode.c \
	build/lang/builtin/modules/array.c \
	build/lang/builtin/modules/dict.c \

OBJS := $(SRCS:.c=.o)

tests: build/tests.o $(OBJS)
	$(CC) $(CFLAGS) -o build/pad_tests $^

pad: build/app.o $(OBJS)
	$(CC) $(CFLAGS) -o build/pad $^

lib: $(OBJS)
	$(CC) $(CFLAGS) -shared -o build/$(OUTLIB) $^

test: build/pad build/pad_tests
	valgrind build/pad_tests
	valgrind build/pad tests/tests.pad

build/app.o: pad/app.c pad/app.h
	$(CC) $(CFLAGS) -c $< -o $@
build/tests.o: tests/tests.c tests/tests.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/error.o: pad/lib/error.c pad/lib/error.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/memory.o: pad/lib/memory.c pad/lib/memory.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/file.o: pad/lib/file.c pad/lib/file.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cstring.o: pad/lib/cstring.c pad/lib/cstring.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/string.o: pad/lib/string.c pad/lib/string.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/unicode.o: pad/lib/unicode.c pad/lib/unicode.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cstring_array.o: pad/lib/cstring_array.c pad/lib/cstring_array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cl.o: pad/lib/cl.c pad/lib/cl.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/format.o: pad/lib/format.c pad/lib/format.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/dict.o: pad/lib/dict.c pad/lib/dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/void_dict.o: pad/lib/void_dict.c pad/lib/void_dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/cmdline.o: pad/lib/cmdline.c pad/lib/cmdline.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/pipe.o: pad/lib/pipe.c pad/lib/pipe.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/term.o: pad/lib/term.c pad/lib/term.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/path.o: pad/lib/path.c pad/lib/path.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lib/socket.o: pad/lib/socket.c pad/lib/socket.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/config.o: pad/core/config.c pad/core/config.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/util.o: pad/core/util.c pad/core/util.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_info.o: pad/core/alias_info.c pad/core/alias_info.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/error_stack.o: pad/core/error_stack.c pad/core/error_stack.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/args.o: pad/core/args.c pad/core/args.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/tokenizer.o: pad/lang/tokenizer.c pad/lang/tokenizer.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/tokens.o: pad/lang/tokens.c pad/lang/tokens.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/nodes.o: pad/lang/nodes.c pad/lang/nodes.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/ast.o: pad/lang/ast.c pad/lang/ast.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/compiler.o: pad/lang/compiler.c pad/lang/compiler.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/traverser.o: pad/lang/traverser.c pad/lang/traverser.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/context.o: pad/lang/context.c pad/lang/context.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/object.o: pad/lang/object.c pad/lang/object.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/object_array.o: pad/lang/object_array.c pad/lang/object_array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/object_dict.o: pad/lang/object_dict.c pad/lang/object_dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/node_array.o: pad/lang/node_array.c pad/lang/node_array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/node_dict.o: pad/lang/node_dict.c pad/lang/node_dict.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/opts.o: pad/lang/opts.c pad/lang/opts.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/scope.o: pad/lang/scope.c pad/lang/scope.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/utils.o: pad/lang/utils.c pad/lang/utils.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/gc.o: pad/lang/gc.c pad/lang/gc.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/kit.o: pad/lang/kit.c pad/lang/kit.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/importer.o: pad/lang/importer.c pad/lang/importer.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/arguments.o: pad/lang/arguments.c pad/lang/arguments.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_node.o: pad/lang/chain_node.c pad/lang/chain_node.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_nodes.o: pad/lang/chain_nodes.c pad/lang/chain_nodes.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_object.o: pad/lang/chain_object.c pad/lang/chain_object.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/chain_objects.o: pad/lang/chain_objects.c pad/lang/chain_objects.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/structs.o: pad/lang/builtin/structs.c 
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/functions.o: pad/lang/builtin/functions.c pad/lang/builtin/functions.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/func_info_array.o: pad/lang/builtin/func_info_array.c pad/lang/builtin/func_info_array.h pad/lang/builtin/func_info.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/module.o: pad/lang/builtin/module.c pad/lang/builtin/module.h 
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/unicode.o: pad/lang/builtin/modules/unicode.c pad/lang/builtin/modules/unicode.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/array.o: pad/lang/builtin/modules/array.c pad/lang/builtin/modules/array.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/modules/dict.o: pad/lang/builtin/modules/dict.c pad/lang/builtin/modules/dict.h
	$(CC) $(CFLAGS) -c $< -o $@
