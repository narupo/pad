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

#include "lib/cstring_array.h"
#include "lib/string.h"
#include "lib/cstring.h"
#include "lib/file.h"
#include "lib/cl.h"
#include "lib/error.h"
#include "lib/cmdline.h"
#include "core/util.h"
#include "core/symlink.h"
#include "core/config.h"
#include "core/alias_info.h"
#include "lang/tokens.h"
#include "lang/tokenizer.h"
#include "lang/nodes.h"
#include "lang/ast.h"
#include "lang/object.h"
#include "lang/object_array.h"
#include "lang/object_dict.h"
#include "lang/opts.h"
