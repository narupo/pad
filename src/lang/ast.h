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
#include <lang/chain_node.h>
#include <lang/chain_nodes.h>

/**
 * constant number of AST
 */
enum {
    AST_ERR_DETAIL_SIZE = 1024, // ast's error message size
};

/**
 * structure of AST
 * this structure using in compiler and traverser modules
 * and this structure has the error handling mechanizm by error_stack variable
 */
struct ast {
    // reference of config (do not delete)
    const config_t *ref_config;

    // reference of tokens with null terminated (do not delete)
    token_t **ref_tokens;

    // refenrece of tokens with null temrinated (do not delete)
    token_t **ref_ptr;

    // root node. compiler parsed
    node_t *root;

    // reference of context. update when traverse tree (do not delete)
    context_t *ref_context;

    // options for builtin opts module
    opts_t *opts;

    // reference to gc (DO NOT DELETE)
    gc_t *ref_gc;

    // number of import level
    int32_t import_level;

    // error stack for errors
    errstack_t *error_stack;

    // if do debug to true
    bool debug;

    // if current context is in loop-statement then store true else false
    // this flag refer from jump statements
    bool is_in_loop;
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
    errstack_pushb(ast->error_stack, fmt, ##__VA_ARGS__)

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
 * clear error stack
 *
 * @param[in] *self
 */
void
ast_clear_errors(ast_t *self);

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
 * get error stack read only
 *
 * @param[in] *self
 *
 * @return pointer to errstack_t
 */
const errstack_t *
ast_getc_error_stack(const ast_t *self);

/**
 * dump ast_t at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
ast_dump(const ast_t *self, FILE *fout);

/**
 * get ast reference of context
 *
 * @param[in] *self
 *
 * @return reference to ast_t (do not delete)
 */
context_t *
ast_get_ref_context(ast_t *self);

/**
 * read token and increment pointer of tokens
 *
 * @param[in] *self
 *
 * @return
 */
token_t *
ast_read_token(ast_t *self);

/**
 * step back pointer of tokens
 *
 * @param[in] *self
 */
void
ast_prev_ptr(ast_t *self);

/**
 * get reference of gc_t
 *
 * @param[in] *self
 *
 * @return reference to gc_t (do not delete)
 */
gc_t *
ast_get_ref_gc(ast_t *self);
