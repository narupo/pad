#pragma once

#include "core/config.h"
#include "lang/types.h"
#include "lang/tokens.h"
#include "lang/nodes.h"
#include "lang/context.h"
#include "lang/opts.h"
#include "lang/node_array.h"
#include "lang/object.h"

enum {
    AST_ERR_DETAIL_SIZE = 1024,
};

struct ast {
    const config_t *config; // read only config
    token_t **tokens; // token list with null at the last
    token_t **ptr; // pointer to tokens
    node_t *root; // pointer to root
    context_t *context; // context. update when traverse tree
    opts_t *opts; // options for builtin opts module
    const object_t *ref_dot_owner; // owner object for dot operator
    char error_detail[AST_ERR_DETAIL_SIZE]; // error detail
    bool debug; // if do debug to true
};

/**
 * 
 *
 * @param[in] *self 
 * @param[in] *node 
 */
void 
ast_del_nodes(const ast_t *self, node_t *node);

/**
 * destruct object
 *
 * @param[in] *self 
 */
void 
ast_del(ast_t *self);

/**
 * construct object
 *
 * @param[in] void 
 *
 * @return 
 */
ast_t * 
ast_new(const config_t *config);

/**
 * 
 *
 * @param[in] *self      
 * @param[in] *move_opts 
 */
void 
ast_move_opts(ast_t *self, opts_t *move_opts);

/**
 * 
 *
 * @param[in] *self 
 *
 * @return 
 */
const node_t * 
ast_getc_root(const ast_t *self);

/**
 * 
 *
 * @param[in] *self 
 * @param[in] *fmt  
 * @param[in] ...   
 *
 * @return 
 */
void 
ast_set_error_detail(ast_t *self, const char *fmt, ...);

/**
 * 
 *
 * @param[in] *self 
 */
void 
ast_clear(ast_t *self);

/**
 * 
 *
 * @param[in] *self 
 *
 * @return 
 */
const char * 
ast_get_error_detail(const ast_t *self);

/**
 * 
 *
 * @param[in] *self 
 *
 * @return 
 */
bool 
ast_has_error(const ast_t *self);

/**
 * 
 *
 * @param[in] *self 
 * @param[in] debug 
 */
void 
ast_set_debug(ast_t *self, bool debug);

