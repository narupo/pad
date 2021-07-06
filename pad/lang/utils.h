#include <assert.h>
#include <pad/lib/error.h>
#include <pad/lib/string.h>
#include <pad/lib/unicode.h>
#include <pad/lib/cstring_array.h>
#include <pad/lang/ast.h>
#include <pad/lang/object.h>
#include <pad/lang/context.h>
#include <pad/lang/nodes.h>
#include <pad/lang/arguments.h>

/*********
* macros *
*********/

#undef Pad_PushBackErrTok
#define Pad_PushBackErrTok(errstack, token, fmt, ...) { \
        const PadTok *t = token; \
        const char *fname = NULL; \
        int32_t lineno = 0; \
        const char *src = NULL; \
        int32_t pos = 0; \
        if (t) { \
            fname = t->program_filename; \
            lineno = t->program_lineno; \
            src = t->program_source; \
            pos = t->program_source_pos; \
        } \
        PadErrStack_PushBack(errstack, fname, lineno, src, pos, fmt, ##__VA_ARGS__); \
    }

#undef Pad_PushBackErrNode
#define Pad_PushBackErrNode(errstack, node, fmt, ...) { \
        const PadNode *n = node; \
        const char *fname = NULL; \
        int32_t lineno = 0; \
        const char *src = NULL; \
        int32_t pos = 0; \
        if (n) { \
            const PadTok *t = n->ref_token; \
            if (t) { \
                fname = t->program_filename; \
                lineno = t->program_lineno; \
                src = t->program_source; \
                pos = t->program_source_pos; \
            } \
        } \
        PadErrStack_PushBack(errstack, fname, lineno, src, pos, fmt, ##__VA_ARGS__); \
    }

/************
* functions *
************/

PadCtx *
Pad_GetCtxByOwns(PadObjAry *ref_owners, PadCtx *def_context);

/**
 * pull-in reference of object by identifier object from varmap of current scope of context
 * return reference to variable
 * if not found object then not push error stack and return NULL
 *
 * @param[in] *idn_obj identifier object
 *
 * @param return NULL or reference to object in varmap in current scope (DO NOT DELETE)
 */
PadObj *
Pad_PullRef(const PadObj *idn_obj);

/**
 * traverse previous context
 */
PadObj *
Pad_PullRefAll(const PadObj *idn_obj);

/**
 * object to string
 * if object is identifier object then pull reference and convert to string
 * if error to set ast error detail
 *
 * @return failed to NULL
 * @return success to pointer to PadStr copied (can delete)
 */
PadStr *
Pad_ObjToString(PadErrStack *err, const PadNode *ref_node, const PadObj *obj);

/**
 * move object at varmap of current scope of context by identifier
 * this function do not increment reference count of object
 */
bool
Pad_MoveObjAtCurVarmap(
    PadErrStack *err,
    const PadNode *ref_node,
    PadCtx *ctx,
    PadObjAry *ref_owners,
    const char *identifier,
    PadObj *move_obj
);

/**
 * set reference of object at varmap of current scope by key
 * this function auto increment reference count of object (ref)
 */
bool
Pad_SetRefAtCurVarmap(
    PadErrStack *err,
    const PadNode *ref_node,
    PadCtx *ctx,
    PadObjAry *ref_owners,
    const char *identifier,
    PadObj *ref
);

bool
Pad_SetRef(PadObjDict *varmap, const char *identifier, PadObj *ref_obj);

/**
 * extract identifier object and index object and etc to reference
 *
 * @return reference to object
 */
PadObj *
Pad_ExtractRefOfObj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
);

PadObj *
Pad_ExtractRefOfObjAll(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
);

/**
 * extract identifier objects
 * return copied object
 *
 * !!! WARNING !!!
 *
 * this function may happen to recusive deep copy loop
 *
 * @return new object
 */
PadObj *
Pad_ExtractCopyOfObj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
);

/**
 * refer index object on context and return reference of refer value
 *
 * @param[in] *ast
 * @param[in] *index_obj
 *
 * @return success to reference to object of index
 * @return failed to NULL
 */
PadObj *
Pad_ReferRingObjWithRef(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *ring_obj
);

/**
 * refer three objects (dot, call, index)
 * this function will be used in loop
 *
 * @param[in] *ast    pointer to PadAST
 * @param[in] *owners owner objects (contain first operand)
 * @param[in] *co     chain object
 *
 * @return success to refer object
 * @return failed to NULL
 */
PadObj *
Pad_ReferChainThreeObjs(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,
    PadChainObj *co
);

/**
 * refer chain call
 *
 * @param[in] *ast    pointer to PadAST
 * @param[in] *owners owner objects (contain first operand)
 * @param[in] *co     chain object
 *
 * @return success to refer object
 * @return failed to NULL
 */
PadObj *
Pad_ReferChainCall(
    PadAST *ref_ast,
    PadErrStack *err,
    const PadNode *ref_node,
    PadGC *ref_gc,
    PadCtx *ref_context,
    PadObjAry *owns,  // TODO: const
    PadChainObj *co
);

PadObj *
Pad_ReferAndSetRef(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *chain_obj,
    PadObj *ref
);

/**
 * dump array object's elements at stdout
 *
 * @param[in] *arrobj
 */
void
Pad_DumpAryObj(const PadObj *arrobj);

/**
 * objectをbool値にする
 *
 * @return true or false
 */
bool
Pad_ParseBool(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
);

/**
 * objectをint値にする
 *
 * @return true or false
 */
PadIntObj
Pad_ParseInt(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
);

/**
 * objectをfloat値にする
 *
 * @return true or false
 */
PadFloatObj
Pad_ParseFloat(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
);

/**
 * if idnobj has in current scope then return true else return false
 *
 * @param[in] *ast
 * @param[in] *idnobj identifier object (type == PAD_OBJ_TYPE__IDENT)
 *
 * @return true or false
 */
bool
Pad_IsVarInCurScope(const PadObj *idnobj);
