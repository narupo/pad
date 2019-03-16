# REFERENCES:
#
# 	https://itchyny.hatenablog.com/entry/20120213/1329135107
#
RM := del
RMDIR := rmdir /s /q
MKDIR_P := mkdir -p
CC := gcc
CFLAGS := -Wall \
	-g \
	-O0 \
	-std=c11 \
	-Wno-unused-function \
	-D_DEBUG \
	-ID:/src/cap/src

all: tests cap

tests: build/tests.o \
	build/lib_error.o \
	build/lib_memory.o \
	build/lib_file.o \
	build/lib_cstring.o \
	build/lib_string.o \
	build/lib_cstring_array.o \
	build/lib_cl.o \
	build/modules_config.o \
	build/modules_util.o \
	build/modules_alias_manager.o \
	build/modules_commands_home.o \
	build/modules_commands_cd.o \
	build/modules_commands_pwd.o \
	build/modules_commands_ls.o \
	build/modules_commands_cat.o \
	build/modules_commands_run.o \
	build/modules_commands_alias.o
	$(CC) $(CFLAGS) -o build/tests \
		build/tests.o \
		build/lib_error.o \
		build/lib_memory.o \
		build/lib_file.o \
		build/lib_cstring.o \
		build/lib_string.o \
		build/lib_cstring_array.o \
		build/lib_cl.o \
		build/modules_config.o \
		build/modules_util.o \
		build/modules_alias_manager.o \
		build/modules_commands_home.o \
		build/modules_commands_cd.o \
		build/modules_commands_pwd.o \
		build/modules_commands_ls.o \
		build/modules_commands_cat.o \
		build/modules_commands_run.o \
		build/modules_commands_alias.o

build/tests.o: src/tests.c src/tests.h
	$(CC) $(CFLAGS) -c $< -o $@

cap: build/app.o \
	build/lib_error.o \
	build/lib_memory.o \
	build/lib_file.o \
	build/lib_cstring.o \
	build/lib_string.o \
	build/lib_cstring_array.o \
	build/lib_cl.o \
	build/modules_config.o \
	build/modules_util.o \
	build/modules_alias_manager.o \
	build/modules_commands_home.o \
	build/modules_commands_cd.o \
	build/modules_commands_pwd.o \
	build/modules_commands_ls.o \
	build/modules_commands_cat.o \
	build/modules_commands_run.o \
	build/modules_commands_alias.o
	$(CC) $(CFLAGS) -o build/cap \
		build/app.o \
		build/lib_error.o \
		build/lib_memory.o \
		build/lib_file.o \
		build/lib_cstring.o \
		build/lib_string.o \
		build/lib_cstring_array.o \
		build/lib_cl.o \
		build/modules_config.o \
		build/modules_util.o \
		build/modules_alias_manager.o \
		build/modules_commands_home.o \
		build/modules_commands_cd.o \
		build/modules_commands_pwd.o \
		build/modules_commands_ls.o \
		build/modules_commands_cat.o \
		build/modules_commands_run.o \
		build/modules_commands_alias.o

build/app.o: src/app.c src/app.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_error.o: src/lib/error.c src/lib/error.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_memory.o: src/lib/memory.c src/lib/memory.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_file.o: src/lib/file.c src/lib/file.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_cstring.o: src/lib/cstring.c src/lib/cstring.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_string.o: src/lib/string.c src/lib/string.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_cstring_array.o: src/lib/cstring_array.c src/lib/cstring_array.h
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_cl.o: src/lib/cl.c src/lib/cl.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_config.o: src/modules/config.c src/modules/config.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_util.o: src/modules/util.c src/modules/util.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_alias_manager.o: src/modules/alias_manager.c src/modules/alias_manager.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_home.o: src/modules/commands/home.c src/modules/commands/home.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_cd.o: src/modules/commands/cd.c src/modules/commands/cd.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_pwd.o: src/modules/commands/pwd.c src/modules/commands/pwd.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_ls.o: src/modules/commands/ls.c src/modules/commands/ls.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_cat.o: src/modules/commands/cat.c src/modules/commands/cat.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_run.o: src/modules/commands/run.c src/modules/commands/run.h
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_alias.o: src/modules/commands/alias.c src/modules/commands/alias.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RMDIR) build



