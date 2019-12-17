/**
 * Cap
 *
 * CL is Command Line.
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#define _GNU_SOURCE 1 /* cap: cl.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "lib/cstring.h"

#if defined(_WIN32) || defined(_WIN64)
# define _CAP_WINDOWS 1
#endif

#ifdef _CAP_WINDOWS

#else
# include <sys/wait.h>
#endif

/********************
* cl option numbers *
********************/

enum {
    CL_DEBUG = (1 << 1),
    CL_WRAP = (1 << 2),
    CL_ESCAPE = (1 << 3),
};

/*****
* cl *
*****/

struct cl;
typedef struct cl cl_t;

/**
 * Destruct cl
 * 
 * @param[in] *self 
 */
void
cl_del(cl_t *self);

/**
 * Destruct cl with move semantics
 * 
 * @param[in] *self 
 * 
 * @return pointer to pointer to dynamic allocate memory of array like a argv. should be free(3)
 */
char **
cl_escdel(cl_t *self);

/**
 * Construct cl
 * 
 * @return success to pointer to dynamic allocate memory of cl
 * @return failed to NULL
 */
cl_t *
cl_new(void);

/**
 * Resize cl
 * 
 * @param[in] *self   
 * @param[in] newcapa number of resize new capacity
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cl_t *
cl_resize(cl_t *self, int32_t newcapa);

/**
 * Push element with copy
 * 
 * @param[in] *self 
 * @param[in] *str string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cl_t *
cl_push(cl_t *self, const char *str);

/**
 * Clear cl
 * 
 * @param[in] *self 
 */
void
cl_clear(cl_t *self);

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
cl_t *
cl_parse_str_opts(cl_t *self, const char *cmdline, int32_t opts);

/**
 * Parse string of command line
 * 
 * @param[in] *self    
 * @param[in] *cmdline string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cl_t *
cl_parse_str(cl_t *self, const char *cmdline);

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
cl_t *
cl_parse_argv_opts(cl_t *self, int argc, char *argv[], int32_t opts);

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
cl_t *
cl_parse_argv(cl_t *self, int argc, char *argv[]);

/**
 * Show cl to stream
 * 
 * @param[in] *self 
 * @param[in] *fout pointer to destruct stream
 */
void
cl_show(const cl_t *self, FILE *fout);

/**
 * Get length of cl
 * 
 * @param[in] *self 
 * 
 * @return number of length of cl
 */
int32_t
cl_len(const cl_t *self);

/**
 * Get capacity of cl
 *
 * @param[in] *self 
 * 
 * @return number of capacity of cl
 */
int32_t
cl_capa(const cl_t *self);

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
cl_getc(const cl_t *self, int32_t idx);

/**
 * Generate string from object
 *
 * @param[in] *self
 *
 * @return success to pointer to strings dynamic allocate memory
 * @return failed to pointer to NULL
 */
char *
cl_to_string(const cl_t *self);
