/**
 * parse command line with pipe (|) and operator (&&, &) and redirect (>)
 *
 * @example
 *
 *     PadCmdline *cmdline = PadCmdline_New();
 *     PadCmdline_Parse(cmdline, "/bin/date-line.py | /bin/lstab.py");
 *     PadCmdline_Del(cmdline);
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
    PAD_CMDLINE_OBJ_TYPE__CMD,
    PAD_CMDLINE_OBJ_TYPE__PIPE,
    PAD_CMDLINE_OBJ_TYPE__AND,
    PAD_CMDLINE_OBJ_TYPE__REDIRECT,
} PadCmdlineObjType;

/**
 * structure of PadCmdlineObj
 * this is element in array of cmdline object
 * DO NOT DELETE command, and cl object
 */
struct PadCmdlineObj {
    PadCmdlineObjType type;
    PadStr *command;
    PadCL *cl;
};

struct PadCmdlineObj;
typedef struct PadCmdlineObj PadCmdlineObj;

struct PadCmdline;
typedef struct PadCmdline PadCmdline;

/*****************
* PadCmdlineObj *
*****************/

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to PadCmdlineObj dynamic allocate memory
 */
void
PadCmdlineObj_Del(PadCmdlineObj *self);

/**
 * construct PadObj
 *
 * @param[in] type number of type of object
 *
 * @return pointer to PadCmdlineObj dynamic allocate memory
 */
PadCmdlineObj *
PadCmdlineObj_New(PadCmdlineObjType type);

/**
 * parse text line
 *
 * @param[in] *self pointer to PadCmdlineObj dynamic allocate memory
 * @param[in] *line pointer to strings
 *
 * @return success to pointer to PadCmdlineObj dynamic allocate memory
 * @return failed to pointer to NULL
 */
PadCmdlineObj *
PadCmdlineObj_Parse(PadCmdlineObj *self, const char *line);

/**********
* cmdline *
**********/

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 */
void
PadCmdline_Del(PadCmdline *self);

/**
 * construct PadObj
 *
 * @return pointer to PadCmdline dynamic allocate memory
 */
PadCmdline *
PadCmdline_New(void);

/**
 * resize objects array
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 * @param[in] capa  number of new capacity
 *
 * @return success to pointer to PadCmdline dynamic allocate memory
 * @return failed to pointer to NULL
 */
PadCmdline *
PadCmdline_Resize(PadCmdline *self, int32_t capa);

/**
 * move back object at array
 *
 * @param[in] *self     pointer to PadCmdline dynamic allocate memory
 * @param[in] *move_obj pointer to object with move semantics
 *
 * @return success to pointer to PadCmdline dynamic allocate memory
 * @return failed to pointer to NULL
 */
PadCmdline *
PadCmdline_MoveBack(PadCmdline *self, PadCmdlineObj *move_obj);

/**
 * if object has error then return true
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 *
 * @return if has error then true
 * @return if not has error then false
 */
bool
PadCmdline_HasErr(const PadCmdline *self);

/**
 * get length of objects array
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 *
 * @return number of length of array
 */
int32_t
PadCmdline_Len(const PadCmdline *self);

/**
 * get read-only object from array
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 * @param[in] index number of index of array
 *
 * @return success to pointer to PadCmdlineObj dynamic allocate memory
 * @return failed to pointer to NULL
 */
const PadCmdlineObj *
PadCmdline_Getc(const PadCmdline *self, int32_t index);

/**
 * parse text line
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 * @param[in] *line pointer to strings
 *
 * @return success to pointer to PadCmdline dynamic allocate memory
 * @return failed to pointer to NULL
 */
PadCmdline *
PadCmdline_Parse(PadCmdline *self, const char *line);

/**
 * clear state
 *
 * @param[in] *self pointer to PadCmdline dynamic allocate memory
 */
void 
PadCmdline_Clear(PadCmdline *self);
