/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: tests: strdup */
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

#include "array.h"
#include "string.h"
#include "file.h"
#include "env.h"
#include "hash.h"
#include "cl.h"
#include "util.h"
#include "error.h"
#include "var.h"
