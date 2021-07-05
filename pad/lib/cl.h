/**
 * Cap
 *
 * CL is Command Line.
 *
 * License: MIT
 *  Author: narupo
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

#include <pad/lib/cstring.h>

#if defined(_WIN32) || defined(_WIN64)
# define PAD_WINDOWS 1
#endif

#ifdef PAD_WINDOWS

#else
# include <sys/wait.h>
#endif

/********************
* cl option numbers *
********************/

enum {
    PAD_CL__DEBUG = (1 << 1),
    PAD_CL__WRAP = (1 << 2),
    PAD_CL__ESCAPE = (1 << 3),
};

/*****
* cl *
*****/

struct PadCL;
typedef struct PadCL PadCL;

/**
 * Destruct PadCL
 *
 * @param[in] *self
 */
void
PadCL_Del(PadCL *self);

/**
 * Destruct PadCL with move semantics
 *
 * @param[in] *self
 *
 * @return pointer to pointer to dynamic allocate memory of array like a argv. should be free(3)
 */
char **
PadCL_EscDel(PadCL *self);

/**
 * Construct PadCL
 *
 * @return success to pointer to dynamic allocate memory of cl
 * @return failed to NULL
 */
PadCL *
PadCL_New(void);

/**
 * Resize cl
 *
 * @param[in] *self
 * @param[in] newcapa number of resize new capacity
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCL *
PadCL_Resize(PadCL *self, int32_t newcapa);

/**
 * Push element with copy
 *
 * @param[in] *self
 * @param[in] *str string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCL *
PadCL_PushBack(PadCL *self, const char *str);

/**
 * Clear cl
 *
 * @param[in] *self
 */
void
PadCL_Clear(PadCL *self);

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
PadCL *
PadCL_ParseStrOpts(PadCL *self, const char *cmdline, int32_t opts);

/**
 * Parse string of command line
 *
 * @param[in] *self
 * @param[in] *cmdline string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCL *
PadCL_ParseStr(PadCL *self, const char *cmdline);

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
PadCL *
PadCL_ParseArgvOpts(PadCL *self, int argc, char *argv[], int32_t opts);

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
PadCL *
PadCL_ParseArgv(PadCL *self, int argc, char *argv[]);

/**
 * Show cl to stream
 *
 * @param[in] *self
 * @param[in] *fout pointer to destruct stream
 */
void
PadCL_Show(const PadCL *self, FILE *fout);

/**
 * Get length of cl
 *
 * @param[in] *self
 *
 * @return number of length of cl
 */
int32_t
PadCL_Len(const PadCL *self);

/**
 * Get capacity of cl
 *
 * @param[in] *self
 *
 * @return number of capacity of cl
 */
int32_t
PadCL_Capa(const PadCL *self);

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
PadCL_Getc(const PadCL *self, int32_t idx);

/**
 * get argv
 *
 * @param[in] *self
 *
 * @return pointer to array
 */
char **
PadCL_GetArgv(const PadCL *self);

/**
 * Generate string from object
 *
 * @param[in] *self
 *
 * @return success to pointer to strings dynamic allocate memory
 * @return failed to pointer to NULL
 */
char *
PadCL_GenStr(const PadCL *self);
