/**
 * parse command line with pipe (|) and operator (&&, &)
 *
 * cmdline_parse(cmdline, "/bin/date-line.py | /bin/lstab.py");
 */
#pragma once

#include <stdint.h>
#include "lib/memory.h"
#include "lib/cl.h"
#include "lib/string.h"

typedef enum {
    CMDLINE_OBJECT_TYPE_CMD = 1,
    CMDLINE_OBJECT_TYPE_PIPE = 2,
} cmdline_object_type_t;

struct cmdline_object {
    cmdline_object_type_t type;
    string_t *command;
    cl_t *cl;
};

struct cmdline_object;
typedef struct cmdline_object cmdline_object_t;

struct cmdline;
typedef struct cmdline cmdline_t;

/*****************
* cmdline_object *
*****************/

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cmdlineobj_del(cmdline_object_t *self);

/**
 * 
 *
 * @param[in]  type 
 *
 * @return 
 */
cmdline_object_t * 
cmdlineobj_new(cmdline_object_type_t type);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *line 
 *
 * @return 
 */
cmdline_object_t * 
cmdlineobj_parse(cmdline_object_t *self, const char *line);

/**********
* cmdline *
**********/

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cmdline_del(cmdline_t *self);

/**
 * 
 *
 * @param[in]  void 
 *
 * @return 
 */
cmdline_t * 
cmdline_new(void);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  capa  
 *
 * @return 
 */
cmdline_t * 
cmdline_resize(cmdline_t *self, int32_t capa);

/**
 * 
 *
 * @param[in]  *self     
 * @param[in]  *move_obj 
 *
 * @return 
 */
cmdline_t * 
cmdline_moveb(cmdline_t *self, cmdline_object_t *move_obj);

/**
 * 
 *
 * @param[in]  *self 
 *
 * @return 
 */
bool 
cmdline_has_error(const cmdline_t *self);

/**
 * 
 *
 * @param[in]  *self 
 *
 * @return 
 */
int32_t 
cmdline_len(const cmdline_t *self);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  index 
 *
 * @return 
 */
const cmdline_object_t * 
cmdline_getc(const cmdline_t *self, int32_t index);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *line 
 *
 * @return 
 */
cmdline_t * 
cmdline_parse(cmdline_t *self, const char *line);

