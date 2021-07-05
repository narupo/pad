/**
 * parse command line with pipe (|) and operator (&&, &) and redirect (>)
 *
 * @example
 *
 *     cmdline_t *cmdline = cmdline_new();
 *     cmdline_parse(cmdline, "/bin/date-line.py | /bin/lstab.py");
 *     cmdline_del(cmdline);
 */
#pragma once

#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/cl.h>
#include <pad/lib/string.h>

/**
 * number of type of cmdline objects
 *
 * the command line
 *
 *     cat text | cat text && make
 *
 * structure is
 *
 *     CMD PIPE CMD AND CMD
 *
 * command line objects
 */
typedef enum {
    CMDLINE_OBJECT_TYPE_CMD,
    CMDLINE_OBJECT_TYPE_PIPE,
    CMDLINE_OBJECT_TYPE_AND,
    CMDLINE_OBJECT_TYPE_REDIRECT,
} cmdline_PadTypeObj;

/**
 * structure of cmdline_object
 * this is element in array of cmdline object
 * DO NOT DELETE command, and cl object
 */
struct cmdline_object {
    cmdline_PadTypeObj type;
    string_t *command;
    PadCL *cl;
};

struct cmdline_object;
typedef struct cmdline_object cmdline_PadObj;

struct cmdline;
typedef struct cmdline cmdline_t;

/*****************
* cmdline_object *
*****************/

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to cmdline_PadObj dynamic allocate memory
 */
void
cmdlinePadObj_Del(cmdline_PadObj *self);

/**
 * construct PadObj
 *
 * @param[in] type number of type of object
 *
 * @return pointer to cmdline_PadObj dynamic allocate memory
 */
cmdline_PadObj *
cmdlinePadObj_New(cmdline_PadTypeObj type);

/**
 * parse text line
 *
 * @param[in] *self pointer to cmdline_PadObj dynamic allocate memory
 * @param[in] *line pointer to strings
 *
 * @return success to pointer to cmdline_PadObj dynamic allocate memory
 * @return failed to pointer to NULL
 */
cmdline_PadObj *
cmdlineobj_parse(cmdline_PadObj *self, const char *line);

/**********
* cmdline *
**********/

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 */
void
cmdline_del(cmdline_t *self);

/**
 * construct PadObj
 *
 * @return pointer to cmdline_t dynamic allocate memory
 */
cmdline_t *
cmdline_new(void);

/**
 * resize objects array
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 * @param[in] capa  number of new capacity
 *
 * @return success to pointer to cmdline_t dynamic allocate memory
 * @return failed to pointer to NULL
 */
cmdline_t *
cmdline_resize(cmdline_t *self, int32_t capa);

/**
 * move back object at array
 *
 * @param[in] *self     pointer to cmdline_t dynamic allocate memory
 * @param[in] *move_obj pointer to object with move semantics
 *
 * @return success to pointer to cmdline_t dynamic allocate memory
 * @return failed to pointer to NULL
 */
cmdline_t *
cmdline_moveb(cmdline_t *self, cmdline_PadObj *move_obj);

/**
 * if object has error then return true
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 *
 * @return if has error then true
 * @return if not has error then false
 */
bool
cmdline_has_error(const cmdline_t *self);

/**
 * get length of objects array
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 *
 * @return number of length of array
 */
int32_t
cmdline_len(const cmdline_t *self);

/**
 * get read-only object from array
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 * @param[in] index number of index of array
 *
 * @return success to pointer to cmdline_PadObj dynamic allocate memory
 * @return failed to pointer to NULL
 */
const cmdline_PadObj *
cmdline_getc(const cmdline_t *self, int32_t index);

/**
 * parse text line
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 * @param[in] *line pointer to strings
 *
 * @return success to pointer to cmdline_t dynamic allocate memory
 * @return failed to pointer to NULL
 */
cmdline_t *
cmdline_parse(cmdline_t *self, const char *line);

/**
 * clear state
 *
 * @param[in] *self pointer to cmdline_t dynamic allocate memory
 */
void 
cmdline_clear(cmdline_t *self);
