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
TARGET := cap

all: $(TARGET)

$(TARGET): build/app.o \
	build/lib_error.o \
	build/lib_memory.o \
	build/lib_file.o \
	build/lib_cstring.o \
	build/lib_string.o \
	build/lib_cstring_array.o \
	build/modules_config.o \
	build/modules_util.o \
	build/modules_cmdargs.o \
	build/modules_commands_home.o \
	build/modules_commands_cd.o \
	build/modules_commands_pwd.o \
	build/modules_commands_ls.o
	$(CC) $(CFLAGS) build/*.o -o build/cap.exe

build/app.o: src/app.c
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_error.o: src/lib/error.c
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_memory.o: src/lib/memory.c
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_file.o: src/lib/file.c
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_cstring.o: src/lib/cstring.c
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_string.o: src/lib/string.c
	$(CC) $(CFLAGS) -c $< -o $@

build/lib_cstring_array.o: src/lib/cstring_array.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_config.o: src/modules/config.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_util.o: src/modules/util.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_cmdargs.o: src/modules/cmdargs.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_home.o: src/modules/commands/home.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_cd.o: src/modules/commands/cd.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_pwd.o: src/modules/commands/pwd.c
	$(CC) $(CFLAGS) -c $< -o $@

build/modules_commands_ls.o: src/modules/commands/ls.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RMDIR) build



