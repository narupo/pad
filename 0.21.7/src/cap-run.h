/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#define _BSD_SOURCE 1 /* cap: cap-run.h: popen */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "array.h"
#include "env.h"
#include "string.h"
#include "util.h"