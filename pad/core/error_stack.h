#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/error.h>

/*********
* macros *
*********/

#define pusherr(fmt, ...) \
    errstack_pushb(self->errstack, fmt, ##__VA_ARGS__)

#define errstack_pushb(self, fmt, ...) \
    _errstack_pushb(self, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/**********
* errelem *
**********/

enum {
    ERRELEM_FILENAME_SIZE = 1024,
    ERRELEM_FUNCNAME_SIZE = 1024,
    ERRELEM_MESSAGE_SIZE = 1024,
};

typedef struct {
    int32_t lineno;
    char filename[ERRELEM_FILENAME_SIZE];
    char funcname[ERRELEM_FUNCNAME_SIZE];
    char message[ERRELEM_MESSAGE_SIZE];
} errelem_t;

/**
 * show element data at stream
 *
 * @param[in]  *self pointer to errelem_t
 * @param[out] fout  destination stream
 */
void
errelem_show(const errelem_t *self, FILE *fout);

/***********
* errstack *
***********/

struct errstack;
typedef struct errstack errstack_t;

/**
 * destruct object
 *
 * @param[in] *self pointer to errstack_t
 */
void
errstack_del(errstack_t *self);

/**
 * construct object
 *
 * @return pointer to errstack_t dynamic allocate memory (do errstack_del)
 */
errstack_t *
errstack_new(void);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return pointer to errstack_t dynamic allocate memory (do errstack_del)
 */
errstack_t *
errstack_deep_copy(const errstack_t *other);

/**
 * push back error stack info
 *
 * @param[in] *self     pointer to errstack_t
 * @param[in] *filename file name
 * @param[in] lineno    line number
 * @param[in] *funcname function name
 * @param[in] *fmt      message format
 * @param[in] ...       message arguments
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
errstack_t *
_errstack_pushb(
    errstack_t *self,
    const char *filename,
    int32_t lineno,
    const char *funcname,
    const char *fmt,
    ...
);

/**
 * get stack element from stack with read-only
 *
 * @param[in] *self pointer to errstack_t
 * @param[in] idx   number of index of stack
 *
 * @return success to pointer to errelem_t
 * @return failed to pointer to NULL
 */
const errelem_t *
errstack_getc(const errstack_t *self, int32_t idx);

/**
 * show stack trace
 *
 * @param[in]  *self pointer to errstack_t
 * @param[out] *fout destination stream
 */
void
errstack_trace(const errstack_t *self, FILE *fout);

/**
 * get length of stack
 *
 * @param[in] *self pointer to errstack_t
 *
 * @return number of length
 */
int32_t
errstack_len(const errstack_t *self);

/**
 * clear state
 *
 * @param[in] *self pointer to errstack_t
 */
void
errstack_clear(errstack_t *self);

/**
 * extend front other error stack at error stack
 *
 * @param[in] *self pointer to errstack_t
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
errstack_t *
errstack_extendf_other(errstack_t *self, const errstack_t *other);

/**
 * extend back other error stack at error stack
 *
 * @param[in] *self pointer to errstack_t
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
errstack_t *
errstack_extendb_other(errstack_t *self, const errstack_t *other);
