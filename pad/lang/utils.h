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

/**
 * get reference of ast by owner object
 *
 * @param[in] *def_ast    default ast
 * @param[in] *ref_owners reference to owners in array
 *
 * @return default ast or owner's ast
 */
ast_t *
get_ast_by_owners(ast_t *def_ast, object_array_t *ref_owners);

context_t *
get_context_by_owners(context_t *def_context, object_array_t *ref_owners);

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
pull_in_ref_by(const object_t *idn_obj);

/**
 * traverse previous context
 */
object_t *
pull_in_ref_by_all(const object_t *idn_obj);

/**
 * object to string
 * if object is identifier object then pull reference and convert to string
 * if error to set ast error detail
 *
 * @param[in] *ast pointer to ast_t
 * @param[in] *obj target object
 *
 * @return failed to NULL
 * @return success to pointer to string_t copied (can delete)
 */
string_t *
obj_to_string(ast_t *ast, const object_t *obj);

/**
 * move object at varmap of current scope of context by identifier
 * this function do not increment reference count of object
 */
void
move_obj_at_cur_varmap(
    errstack_t *err,
    context_t *ctx,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *move_obj
);

/**
 * set reference of object at varmap of current scope by key
 * this function auto increment reference count of object (ref)
 *
 * @param[in] *ast        pointer to ast_t
 * @param[in] *ref_owners reference to owners in array
 * @param[in] *identifier key of dict item
 * @param[in] *ref        reference to object
 */
void
set_ref_at_cur_varmap(
    errstack_t *err,
    context_t *ctx,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *ref
);

/**
 * extract identifier object and index object and etc to reference
 *
 * @param[in] *ast
 * @param[in] *obj
 *
 * @return reference to object
 */
object_t *
extract_ref_of_obj(ast_t *ast, object_t *obj);

/**
 * extract identifier objects
 * return copied object
 *
 * @return new object
 */
object_t *
extract_copy_of_obj(ast_t *ast, object_t *obj);

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
refer_chain_obj_with_ref(ast_t *ast, object_t *chain_obj);

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
refer_chain_three_objs(ast_t *ast, object_array_t *owners, chain_object_t *co);

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
refer_chain_call(ast_t *ast, object_array_t *owners, chain_object_t *co);

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
 * @param[in] *ast
 * @param[in] *obj
 *
 * @return true or false
 */
bool
parse_bool(ast_t *ast, object_t *obj);

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
