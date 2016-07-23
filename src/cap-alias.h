#ifndef CAP_ALIAS_H
#define CAP_ALIAS_H

#define _GNU_SOURCE 1 /* in cap-alias.h for strdup */
#include <stdio.h>

#include "error.h"
#include "hash.h"
#include "file.h"

#include "alias.h"

/**
 *
 * @return string pointer to dynamic allocate memory
 */
char *
cap_alcmd(const char *name);

#endif
