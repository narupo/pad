#pragma once

#include <core/config.h>
#include <core/error_stack.h>
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
    const config_t *ref_config; // read only config
    token_t **tokens; // token list with null at the last (DO NOT DELETE)
    token_t **ptr; // pointer to tokens for tokenizer (DO NOT DELETE)
    node_t *root; // pointer to root for compiler and traverser
    context_t *context; // context. update when traverse tree (DO NOT DELETE)
    opts_t *opts; // options for builtin opts module
    gc_t *ref_gc; // reference to gc (DO NOT DELETE)
    object_t *ref_dot_owner; // owner object for dot operator (owner.right_hand["key"]) for traverser (DO NOT DELETE)
    int32_t import_level; // number of import level
    errstack_t *error_stack; // error stack for errors
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
 * @param[in] *ref_config pointer to read-only config_t
 *
 * @return pointer to ast_t dynamic allocate memory (do ast_del)
 */
ast_t *
ast_new(const config_t *ref_config);

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
 * push back error at ast error stack
 *
 * @param[in] ast pointer to ast_t
 * @param[in] fmt format string (const char *)
 * @param[in] ... arguments of format
 */
#define ast_pushb_error(ast, fmt, ...) \
    errstack_pushb(ast->error_stack, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/**
 * clear ast state (will call ast_del_nodes)
 *
 * @param[in] *self pointer to ast_t
 */
void
ast_clear(ast_t *self);

/**
 * get first error message from error stack
 *
 * @param[in] *self
 *
 * @return if has error stack then return pointer to message of first error
 * @return if not has error stack then return NULL
 */
const char *
ast_getc_first_error_message(const ast_t *self);

/**
 * get last error message from error stack
 *
 * @param[in] *self
 *
 * @return if has error stack then return pointer to message of last error
 * @return if not has error stack then return NULL
 */
const char *
ast_getc_last_error_message(const ast_t *self);

/**
 * if ast has error stack then return true else return false
 *
 * @param[in] *self
 *
 * @return if has error then true else false
 */
bool
ast_has_errors(const ast_t *self);

/**
 * set debug mode
 * debug of argument is true to debug mode false to non debug mode
 *
 * @param[in] *self pointer to ast_t
 * @param[in] debug debug mode
 */
void
ast_set_debug(ast_t *self, bool debug);

/**
 * trace error stack at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
ast_trace_error_stack(const ast_t *self, FILE *fout);

/**
 * dump ast_t at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
ast_dump(const ast_t *self, FILE *fout);
