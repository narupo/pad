#pragma once

#include <pad/core/config.h>
#include <pad/core/error_stack.h>
#include <pad/lang/types.h>
#include <pad/lang/tokens.h>
#include <pad/lang/nodes.h>
#include <pad/lang/context.h>
#include <pad/lang/opts.h>
#include <pad/lang/node_array.h>
#include <pad/lang/nodes.h>
#include <pad/lang/object.h>
#include <pad/lang/gc.h>
#include <pad/lang/chain_node.h>
#include <pad/lang/chain_nodes.h>
#include <pad/lang/importer.h>

/**
 * constant number of AST
 */
enum {
    PAD_AST__ERR_DETAIL_SIZE = 1024, // ast's error message size
    PAD_AST__ERR_TOKENS_SIZE = 256,
};

/**
 * structure of AST
 * this structure using in compiler and traverser modules
 * and this structure has the error handling mechanizm by error_stack variable
 */
struct PadAst {
    // reference of config (do not delete)
    const PadConfig *ref_config;

    // reference of tokens with null terminated (do not delete)
    PadTok **ref_tokens;

    // refenrece of tokens with null temrinated (do not delete)
    PadTok **ref_ptr;

    // root node. compiler parsed
    PadNode *root;

    // reference of context. update when traverse tree (do not delete)
    PadCtx *ref_context;

    // options for builtin opts module
    PadOpts *opts;

    // reference to gc (DO NOT DELETE)
    PadGC *ref_gc;

    // error stack for errors
    PadErrStack *error_stack;

    // error tokens for display error to developer
    PadTok *error_tokens[PAD_AST__ERR_TOKENS_SIZE];
    int32_t error_tokens_pos;

    // number of import level
    int32_t import_level;

    // if do debug to true
    bool debug;

    // if current context is in loop-statement then store true else false
    // this flag refer from jump statements
    bool is_in_loop;

    PadImporterFixPathFunc importer_fix_path;
};

/**
 * delete node tree
 *
 * @param[in] *self pointer to PadAST
 * @param[in] *node start node
 */
void
PadAst_DelNodes(const PadAST *self, PadNode *node);

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to PadAST
 */
void
PadAst_Del(PadAST *self);

/**
 * construct PadObj
 *
 * @param[in] *ref_config pointer to read-only PadConfig
 *
 * @return pointer to PadAST dynamic allocate memory (do PadAst_Del)
 */
PadAST *
PadAst_New(const PadConfig *ref_config);

PadAST *
PadAst_DeepCopy(const PadAST *other);

PadAST *
PadAst_ShallowCopy(const PadAST *other);

/**
 * move opts at ast
 *
 * @param[in] *self      pointer to PadAST
 * @param[in] *move_opts pointer to PadOpts with move semantics
 */
void
PadAst_MoveOpts(PadAST *self, PadOpts *move_opts);

void
PadAst_SetRefCtx(PadAST *ast, PadCtx *ref_context);

void
PadAst_SetRefGc(PadAST *ast, PadGC *ref_gc);

/**
 * get root node read-only
 *
 * @param[in] *self pointer to PadAST
 *
 * @return pointer to PadNode root
 */
const PadNode *
PadAst_GetcRoot(const PadAST *self);

/**
 * push back error at ast error stack
 *
 * @param[in] ast    pointer to PadAST
 * @param[in] fname  file name of current module
 * @param[in] lineno line number of current module
 * @param[in] src    source string of current module
 * @param[in] pos    number of position in src
 * @param[in] fname  file name of module
 * @param[in] fmt    format string (const char *)
 * @param[in] ...   arguments of format
 */
#define PadAst_PushBackErr(ast, fname, lineno, src, pos, fmt, ...) \
    PadErrStack_PushBack(ast->error_stack, fname, lineno, src, pos, fmt, ##__VA_ARGS__)

/**
 * clear ast state (will call PadAst_DelNodes)
 *
 * @param[in] *self pointer to PadAST
 */
void
PadAst_Clear(PadAST *self);

/**
 * get first error message from error stack
 *
 * @param[in] *self
 *
 * @return if has error stack then return pointer to message of first error
 * @return if not has error stack then return NULL
 */
const char *
PadAst_GetcFirstErrMsg(const PadAST *self);

/**
 * get last error message from error stack
 *
 * @param[in] *self
 *
 * @return if has error stack then return pointer to message of last error
 * @return if not has error stack then return NULL
 */
const char *
PadAst_GetcLastErrMsg(const PadAST *self);

/**
 * if ast has error stack then return true else return false
 *
 * @param[in] *self
 *
 * @return if has error then true else false
 */
bool
PadAst_HasErrs(const PadAST *self);

/**
 * clear error stack
 *
 * @param[in] *self
 */
void
PadAst_ClearErrs(PadAST *self);

/**
 * set debug mode
 * debug of argument is true to debug mode false to non debug mode
 *
 * @param[in] *self pointer to PadAST
 * @param[in] debug debug mode
 */
void
PadAst_SetDebug(PadAST *self, bool debug);

/**
 * trace error stack at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadAst_TraceErr(const PadAST *self, FILE *fout);

/**
 * get error stack read only
 *
 * @param[in] *self
 *
 * @return pointer to PadErrStack
 */
const PadErrStack *
PadAst_GetcErrStack(const PadAST *self);

/**
 * dump PadAST at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadAst_Dump(const PadAST *self, FILE *fout);

/**
 * get ast reference of context
 *
 * @param[in] *self
 *
 * @return reference to PadAST (do not delete)
 */
PadCtx *
PadAst_GetRefCtx(PadAST *self);

/**
 * read token and increment pointer of tokens
 *
 * @param[in] *self
 *
 * @return
 */
PadTok *
PadAst_ReadTok(PadAST *self);

/**
 * step back pointer of tokens
 *
 * @param[in] *self
 */
void
PadAst_PrevPtr(PadAST *self);

/**
 * get reference of PadGC
 *
 * @param[in] *self
 *
 * @return reference to PadGC (do not delete)
 */
PadGC *
PadAst_GetRefGc(PadAST *self);

PadAST *
PadAst_PushBackErrTok(PadAST *self, PadTok *ref_token);
