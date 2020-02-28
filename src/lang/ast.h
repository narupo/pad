#pragma once

#include <core/config.h>
#include <lang/types.h>
#include <lang/tokens.h>
#include <lang/nodes.h>
#include <lang/context.h>
#include <lang/opts.h>
#include <lang/node_array.h>
#include <lang/object.h>
#include <lang/gc.h>

/**
 * constant number of AST
 */
enum {
    AST_ERR_DETAIL_SIZE = 1024, // ast's error message size
};

/**
 * structure of AST
 * this structure using in compiler and traverser modules
 * and this structure has error handling mechanizm by error_detail variable
 */
struct ast {
    const config_t *config; // read only config
    token_t **tokens; // token list with null at the last (DO NOT DELETE)
    token_t **ptr; // pointer to tokens for tokenizer (DO NOT DELETE)
    node_t *root; // pointer to root for compiler and traverser
    context_t *context; // context. update when traverse tree (DO NOT DELETE)
    opts_t *opts; // options for builtin opts module
    gc_t *ref_gc; // reference to gc (DO NOT DELETE)
    object_t *ref_dot_owner; // owner object for dot operator (owner.right_hand["key"]) for traverser (DO NOT DELETE)
    char error_detail[AST_ERR_DETAIL_SIZE]; // error detail
    bool debug; // if do debug to true
};

/**
 * delete node tree
 *
 * @param[in] *self pointer to ast_t
 * @param[in] *node start node 
 */
void 
ast_del_nodes(const ast_t *self, node_t *node);

/**
 * destruct object
 *
 * @param[in] *self pointer to ast_t
 */
void 
ast_del(ast_t *self);

/**
 * construct object
 *
 * @param[in] *config pointer to read-only config_t
 *
 * @return pointer to ast_t dynamic allocate memory (do ast_del)
 */
ast_t * 
ast_new(const config_t *config);

/**
 * move opts at ast
 *
 * @param[in] *self      pointer to ast_t      
 * @param[in] *move_opts pointer to opts_t with move semantics
 */
void 
ast_move_opts(ast_t *self, opts_t *move_opts);

/**
 * get root node read-only
 *
 * @param[in] *self pointer to ast_t
 *
 * @return pointer to node_t root
 */
const node_t * 
ast_getc_root(const ast_t *self);

/**
 * set error message at ast
 *
 * @param[in] *self pointer to ast_t
 * @param[in] *fmt  format strings
 * @param[in] ...   arguments
 */
void 
ast_set_error_detail(ast_t *self, const char *fmt, ...);

/**
 * clear ast state (will call ast_del_nodes)
 *
 * @param[in] *self pointer to ast_t
 */
void 
ast_clear(ast_t *self);

/**
 * get error message from ast read-only
 *
 * @param[in] *self pointer to ast_t
 *
 * @return pointer to error message
 */
const char * 
ast_get_error_detail(const ast_t *self);

/**
 * if ast has error state then return true else return false
 *
 * @param[in] *self pointer to ast_t
 *
 * @return if has error then true else false
 */
bool 
ast_has_error(const ast_t *self);

/**
 * set debug mode
 * debug of argument is true to debug mode false to non debug mode 
 *
 * @param[in] *self pointer to ast_t
 * @param[in] debug debug mode
 */
void 
ast_set_debug(ast_t *self, bool debug);
