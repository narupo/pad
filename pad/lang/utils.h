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

#undef pushb_error_token
#define pushb_error_token(errstack, token, fmt, ...) { \
        const token_t *t = token; \
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

#undef pushb_error_node
#define pushb_error_node(errstack, node, fmt, ...) { \
        const node_t *n = node; \
        const char *fname = NULL; \
        int32_t lineno = 0; \
        const char *src = NULL; \
        int32_t pos = 0; \
        if (n) { \
            const token_t *t = n->ref_token; \
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

context_t *
get_context_by_owners(object_array_t *ref_owners, context_t *def_context);

/**
 * pull-in reference of object by identifier object from varmap of current scope of context
 * return reference to variable
 * if not found object then not push error stack and return NULL
 *
 * @param[in] *idn_obj identifier object
 *
 * @param return NULL or reference to object in varmap in current scope (DO NOT DELETE)
 */
object_t *
pull_ref(const object_t *idn_obj);

/**
 * traverse previous context
 */
object_t *
pull_ref_all(const object_t *idn_obj);

/**
 * object to string
 * if object is identifier object then pull reference and convert to string
 * if error to set ast error detail
 *
 * @return failed to NULL
 * @return success to pointer to string_t copied (can delete)
 */
string_t *
obj_to_string(PadErrStack *err, const node_t *ref_node, const object_t *obj);

/**
 * move object at varmap of current scope of context by identifier
 * this function do not increment reference count of object
 */
bool
move_obj_at_cur_varmap(
    PadErrStack *err,
    const node_t *ref_node,
    context_t *ctx,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *move_obj
);

/**
 * set reference of object at varmap of current scope by key
 * this function auto increment reference count of object (ref)
 */
bool
set_ref_at_cur_varmap(
    PadErrStack *err,
    const node_t *ref_node,
    context_t *ctx,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *ref
);

bool
set_ref(object_dict_t *varmap, const char *identifier, object_t *ref_obj);

/**
 * extract identifier object and index object and etc to reference
 *
 * @return reference to object
 */
object_t *
extract_ref_of_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *obj
);

object_t *
extract_ref_of_obj_all(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *obj
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
object_t *
extract_copy_of_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *obj
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
object_t *
refer_chain_obj_with_ref(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *chain_obj
);

/**
 * refer three objects (dot, call, index)
 * this function will be used in loop
 *
 * @param[in] *ast    pointer to ast_t
 * @param[in] *owners owner objects (contain first operand)
 * @param[in] *co     chain object
 *
 * @return success to refer object
 * @return failed to NULL
 */
object_t *
refer_chain_three_objs(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_array_t *owns,
    chain_object_t *co
);

/**
 * refer chain call
 *
 * @param[in] *ast    pointer to ast_t
 * @param[in] *owners owner objects (contain first operand)
 * @param[in] *co     chain object
 *
 * @return success to refer object
 * @return failed to NULL
 */
object_t *
refer_chain_call(
    ast_t *ref_ast,
    PadErrStack *err,
    const node_t *ref_node,
    gc_t *ref_gc,
    context_t *ref_context,
    object_array_t *owns,  // TODO: const
    chain_object_t *co
);

object_t *
refer_and_set_ref(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *chain_obj,
    object_t *ref
);

/**
 * dump array object's elements at stdout
 *
 * @param[in] *arrobj
 */
void
dump_array_obj(const object_t *arrobj);

/**
 * objectをbool値にする
 *
 * @return true or false
 */
bool
parse_bool(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *obj
);

/**
 * objectをint値にする
 *
 * @return true or false
 */
objint_t
parse_int(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *obj
);

/**
 * objectをfloat値にする
 *
 * @return true or false
 */
objfloat_t
parse_float(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    context_t *ref_context,
    const node_t *ref_node,
    object_t *obj
);

/**
 * if idnobj has in current scope then return true else return false
 *
 * @param[in] *ast
 * @param[in] *idnobj identifier object (type == OBJ_TYPE_IDENTIFIER)
 *
 * @return true or false
 */
bool
is_var_in_cur_scope(const object_t *idnobj);
