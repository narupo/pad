/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1 /* cap: tests: strdup */
#endif

#if defined(_WIN32) || defined(_WIN64)
# define _TESTS_WINDOWS
#endif

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

#include "modules/util.h"

#include "modules/lang/tokens.h"
#include "modules/lang/tokenizer.h"
#include "modules/lang/nodes.h"
#include "modules/lang/ast.h"
