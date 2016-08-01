#ifndef ARGS_H
#define ARGS_H

#define _GNU_SOURCE 1 /* args.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
	/* 0 */ ARG_UNKNOWN,
	/* 1 */ ARG_ARGUMENT,
	/* 2 */ ARG_SHORTOPT,
	/* 3 */ ARG_SHORTOPTASS, // ASS ... Assignment
	/* 4 */ ARG_SHORTOPTS,
	/* 5 */ ARG_SHORTOPTSASS,
	/* 6 */ ARG_LONGOPT,
	/* 7 */ ARG_LONGOPTASS,
} cap_argtype_t;

// cap_arg

struct cap_arg;

void
cap_argdel(struct cap_arg *self);

char *
cap_argescdel(struct cap_arg *self);

struct cap_arg *
cap_argnew(void);

struct cap_arg *
cap_argnewother(const struct cap_arg *other);

cap_argtype_t
cap_argtype(const struct cap_arg *self);

const char *
cap_argvaluec(const struct cap_arg *self);

struct cap_arg *
cap_argparse(struct cap_arg *self, const char *src);

void
cap_argshow(const struct cap_arg *self, FILE *fout);

const char *
cap_argwrapvalue(struct cap_arg *self, int wrpch);

// cap_args

struct cap_args;

void
cap_argsdel(struct cap_args *self);

char **
cap_argsescdel(struct cap_args *self);

struct cap_args *
cap_argsnew(void);

struct cap_args *
cap_argsclear(struct cap_args *self);

struct cap_args *
cap_argsresize(struct cap_args *self, int newcapa);

struct cap_args *
cap_argsmove(struct cap_args *self, struct cap_arg *arg);

struct cap_args *
cap_argsparse(struct cap_args *self, int argc, char *argv[]);

void
cap_argsshow(const struct cap_args *self, FILE *fout);

int
cap_argslen(const struct cap_args *self);

int
cap_argscapa(const struct cap_args *self);

struct cap_arg *
cap_argsget(struct cap_args *self, int idx);

const struct cap_arg *
cap_argsgetc(const struct cap_args *self, int idx);

#endif
