#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/error.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/string.h>
#include <pad/lang/tokens.h>

/*********
* macros *
*********/

#define Pad_PushErr(fmt, ...) \
    PadErrStack_PushBack(self->errstack, NULL, 0, NULL, 0, fmt, ##__VA_ARGS__)

#define PadErrStack_PushBack(stack, prog_fname, prog_lineno, prog_src, prog_src_pos, fmt, ...) \
    _PadErrStack_PushBack( \
        stack, \
        prog_fname, \
        prog_lineno, \
        prog_src, \
        prog_src_pos, \
        __FILE__, \
        __LINE__, \
        __func__, \
        fmt, \
        ##__VA_ARGS__ \
    )

#define PadErrStack_Add(stack, fmt, ...) \
    _PadErrStack_PushBack( \
        stack, \
        NULL, \
        0, \
        NULL, \
        0, \
        __FILE__, \
        __LINE__, \
        __func__, \
        fmt, \
        ##__VA_ARGS__ \
    )

/**********
* errelem *
**********/

enum {
    PAD_ERRELEM_MESSAGE_SIZE = 1024,
};

typedef struct {
    const char *program_filename;
    const char *program_source;
    const char *filename;
    const char *funcname;
    int32_t program_lineno;
    int32_t program_source_pos;
    int32_t lineno;
    char message[PAD_ERRELEM_MESSAGE_SIZE];
} PadErrElem;

/**
 * show element data at stream
 *
 * @param[in]  *self pointer to PadErrElem
 * @param[out] fout  destination stream
 */
void
PadErrElem_Show(const PadErrElem *self, FILE *fout);

/***********
* errstack *
***********/

struct PadErrStack;
typedef struct PadErrStack PadErrStack;

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to PadErrStack
 */
void
PadErrStack_Del(PadErrStack *self);

/**
 * construct PadObj
 *
 * @return pointer to PadErrStack dynamic allocate memory (do PadErrStack_Del)
 */
PadErrStack *
PadErrStack_New(void);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return pointer to PadErrStack dynamic allocate memory (do PadErrStack_Del)
 */
PadErrStack *
PadErrStack_DeepCopy(const PadErrStack *other);

PadErrStack *
PadErrStack_ShallowCopy(const PadErrStack *other);

/**
 * push back error stack info
 *
 * @param[in] *self     pointer to PadErrStack
 * @param[in] *filename file name
 * @param[in] lineno    line number
 * @param[in] *funcname function name
 * @param[in] *fmt      message format
 * @param[in] ...       message arguments
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
PadErrStack *
_PadErrStack_PushBack(
    PadErrStack *self,
    const char *program_filename,
    int32_t program_lineno,
    const char *program_source,
    int32_t program_source_pos,
    const char *filename,
    int32_t lineno,
    const char *funcname,
    const char *fmt,
    ...
);

/**
 * get stack element from stack with read-only
 *
 * @param[in] *self pointer to PadErrStack
 * @param[in] idx   number of index of stack
 *
 * @return success to pointer to PadErrElem
 * @return failed to pointer to NULL
 */
const PadErrElem *
PadErrStack_Getc(const PadErrStack *self, int32_t idx);

/**
 * show stack trace
 *
 * @param[in]  *self pointer to PadErrStack
 * @param[out] *fout destination stream
 */
void
PadErrStack_Trace(const PadErrStack *self, FILE *fout);

void
PadErrStack_TraceDebug(const PadErrStack *self, FILE *fout);

void
PadErrStack_TraceSimple(const PadErrStack *self, FILE *fout);

/**
 * get length of stack
 *
 * @param[in] *self pointer to PadErrStack
 *
 * @return number of length
 */
int32_t
PadErrStack_Len(const PadErrStack *self);

/**
 * clear state
 *
 * @param[in] *self pointer to PadErrStack
 */
void
PadErrStack_Clear(PadErrStack *self);

/**
 * extend front other error stack at error stack
 *
 * @param[in] *self pointer to PadErrStack
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadErrStack *
PadErrStack_ExtendFrontOther(PadErrStack *self, const PadErrStack *other);

/**
 * extend back other error stack at error stack
 *
 * @param[in] *self pointer to PadErrStack
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadErrStack *
PadErrStack_ExtendBackOther(PadErrStack *self, const PadErrStack *other);

string_t *
PadErrStack_TrimAround(const char *src, int32_t pos);
