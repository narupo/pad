/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#pragma once

#if defined(_WIN32) || defined(_WIN64)
# define PAD_TESTS__WINDOWS
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

#include <pad/lib/cstring_array.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lib/file.h>
#include <pad/lib/cl.h>
#include <pad/lib/void_dict.h>
#include <pad/lib/error.h>
#include <pad/lib/cmdline.h>
#include <pad/lib/unicode.h>
#include <pad/lib/path.h>
#include <pad/lib/unicode_path.h>
#include <pad/core/util.h>
#include <pad/core/config.h>
#include <pad/core/alias_info.h>
#include <pad/core/error_stack.h>
#include <pad/lang/types.h>
#include <pad/lang/tokens.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/nodes.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/object.h>
#include <pad/lang/object_array.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/opts.h>
#include <pad/lang/gc.h>
#include <pad/lang/builtin/modules/alias.h>
#include <pad/lang/builtin/modules/opts.h>
