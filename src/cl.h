#ifndef CL_H
#define CL_H

#define _GNU_SOURCE 1 /* cap: cl.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

enum {
	CL_WRAP = (1 << 1),
	CL_ESCAPE = (1 << 2),
};

struct cap_cl;

void
cap_cldel(struct cap_cl *self);

char **
cap_clescdel(struct cap_cl *self);

struct cap_cl *
cap_clnew(void);

struct cap_cl *
cap_clresize(struct cap_cl *self, int newcapa);

struct cap_cl *
cap_clpush(struct cap_cl *self, const char *str);

void
cap_clclear(struct cap_cl *self);

struct cap_cl *
cap_clparsestropts(struct cap_cl *self, const char *drtsrc, int opts);

struct cap_cl *
cap_clparsestr(struct cap_cl *self, const char *drtcl);

struct cap_cl *
cap_clparseargvopts(struct cap_cl *self, int argc, char *argv[], int opts);

struct cap_cl *
cap_clparseargv(struct cap_cl *self, int argc, char *argv[]);

void
cap_clshow(const struct cap_cl *self, FILE *fout);

int
cap_cllen(const struct cap_cl *self);

#endif
