#ifndef UTIL_H
#define UTIL_H

#include "define.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

/**
 * Get length of static array
 *
 * @param[in] static array
 * @return number of length of array
 */
#define NUMOF(array) (sizeof(array)/sizeof(array[0]))

/**
 *
 *
 * @param[]
 *
 * @return
 */
void
free_argv(int argc, char** argv);

#endif
