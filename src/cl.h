/**
 * Cap
 *
 * CL is Command Line.
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
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

/************************
* cap_cl option numbers *
************************/

enum {
	CL_DEBUG = (1 << 1),
	CL_WRAP = (1 << 2),
	CL_ESCAPE = (1 << 3),
};

/*********
* cap_cl *
*********/

struct cap_cl;

/**
 * Destruct cl
 * 
 * @param[in] *self 
 */
void
cap_cldel(struct cap_cl *self);

/**
 * Destruct cl with move semantics
 * 
 * @param[in] *self 
 * 
 * @return pointer to pointer to dynamic allocate memory of array like a argv. User should be free(3)
 */
char **
cap_clescdel(struct cap_cl *self);

/**
 * Construct cl
 * 
 * @return success to pointer to dynamic allocate memory of cl
 * @return failed to NULL
 */
struct cap_cl *
cap_clnew(void);

/**
 * Resize cl
 * 
 * @param[in] *self   
 * @param[in] newcapa number of resize new capacity
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_cl *
cap_clresize(struct cap_cl *self, int newcapa);

/**
 * Push element with copy
 * 
 * @param[in] *self 
 * @param[in] *str string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_cl *
cap_clpush(struct cap_cl *self, const char *str);

/**
 * Clear cl
 * 
 * @param[in] *self 
 */
void
cap_clclear(struct cap_cl *self);

/**
 * Parse string of command line by options
 * 
 * @param[in] *self   
 * @param[in] *cmdline string
 * @param[in] opts number of cl options
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_cl *
cap_clparsestropts(struct cap_cl *self, const char *cmdline, int opts);

/**
 * Parse string of command line
 * 
 * @param[in] *self    
 * @param[in] *cmdline string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_cl *
cap_clparsestr(struct cap_cl *self, const char *cmdline);

/**
 * Parse argv by options
 * 
 * @param[in] *self
 * @param[in] argc number of length of argv
 * @param[in] *argv[] pointer to pointer to string
 * @param[in] opts number of cl options
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_cl *
cap_clparseargvopts(struct cap_cl *self, int argc, char *argv[], int opts);

/**
 * Parse argv
 * 
 * @param[in] *self   
 * @param[in] argc number of lenght of argv
 * @param[in] *argv[] pointer to pointer to string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_cl *
cap_clparseargv(struct cap_cl *self, int argc, char *argv[]);

/**
 * Show cl to stream
 * 
 * @param[in] *self 
 * @param[in] *fout pointer to destruct stream
 */
void
cap_clshow(const struct cap_cl *self, FILE *fout);

/**
 * Get length of cl
 * 
 * @param[in] *self 
 * 
 * @return number of length of cl
 */
int
cap_cllen(const struct cap_cl *self);

/**
 * Get capacity of cl
 *
 * @param[in] *self 
 * 
 * @return number of capacity of cl
 */
int
cap_clcapa(const struct cap_cl *self);

/**
 * Get element in cl
 *
 * @param[in] *self
 * @param[in] idx index of array
 *
 * @return success to pointer to element
 * @return failed to NULL
 */
const char *
cap_clgetc(const struct cap_cl *self, int idx);

#endif
