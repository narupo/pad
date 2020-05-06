/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#if defined(_WIN32) || defined(_WIN64)
# define _TESTS_WINDOWS
#endif

#define _SVID_SOURCE 1 /* cap: tests: strdup */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include <lib/cstring_array.h>
#include <lib/string.h>
#include <lib/cstring.h>
#include <lib/file.h>
#include <lib/cl.h>
#include <lib/error.h>
#include <lib/cmdline.h>
#include <pwd/pwd.h>
#include <cd/cd.h>
#include <ls/ls.h>
#include <cat/cat.h>
#include <core/util.h>
#include <core/symlink.h>
#include <core/config.h>
#include <core/alias_info.h>
#include <core/error_stack.h>
#include <lang/types.h>
#include <lang/tokens.h>
#include <lang/tokenizer.h>
#include <lang/nodes.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/object.h>
#include <lang/object_array.h>
#include <lang/object_dict.h>
#include <lang/opts.h>
#include <lang/gc.h>
#include <lang/builtin/function.h>
#include <lang/builtin/modules/alias.h>
#include <lang/builtin/modules/opts.h>
