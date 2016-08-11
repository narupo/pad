#ifndef CL_H
#define CL_H

#define _GNU_SOURCE 1 /* cap: cl.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cap_cl;

void
cap_cldel(struct cap_cl *self);

struct cap_cl *
cap_clnew(void);

#endif

