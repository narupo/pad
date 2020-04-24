#include <assert.h>
#include <lib/error.h>
#include <lib/string.h>
#include <lib/cstring_array.h>
#include <lang/ast.h>
#include <lang/object.h>
#include <lang/context.h>
#include <lang/nodes.h>

/**
 * get variable reference from varmap of current scope of context
 *
 * @param[in] *ast        pointer to ast_t
 * @param[in] *identifier strings
 *
 * @return NULL | reference to object_t (DO NOT DELETE)
 */
object_t *
get_var_ref(ast_t *ast, const char *identifier);

/**
 * pull-in reference of object by identifier object from varmap of current scope of context
 * return reference to variable
 *
 * @param[in] *ast     pointer to ast_t
 * @param[in] *idn_obj identifier object
 *
 * @param return NULL or reference to object in varmap in current scope (DO NOT DELETE)
 */
object_t *
pull_in_ref_by(ast_t *ast, const object_t *idn_obj);

/**
 * see at ast->ref_dot_owner
 * if ast has ref_dot_owner this function see it
 *
 * @param[in] *ast
 * @param[in] *idn_obj
 *
 * @return
 */
object_t *
pull_in_ref_by_owner(ast_t *ast, const object_t *idn_obj);

/**
 * copy value of index object
 *
 * @param[in] *ast       pointer to ast_t
 * @param[in] *index_obj index object
 *
 * @return copy object (can delete)
 */
object_t *
copy_value_of_index_obj(ast_t *ast, const object_t *index_obj);

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
 * copy object value
 * if object is identifier object or index object then copy that value
 *
 * @param[in] *ast pointer to ast_t
 * @param[in] *obj target object
 *
 * @return NULL | copied object (can delete)
 */
object_t *
copy_object_value(ast_t *ast, const object_t *obj);

/**
 * set move object at varmap of current scope of context by identifier
 * this function do not increment reference count of object
 *
 * @param[in] *ast        pointer to ast_t
 * @param[in] *identifier identifier string
 * @param[in] *move_obj   object with move semantics
 */
void
move_obj_at_cur_varmap(ast_t *ast, const char *identifier, object_t *move_obj);

/**
 * set reference of object at varmap of current scope by key
 * this function auto increment reference count of object
 *
 * @param[in] *ast        pointer to ast_t
 * @param[in] *identifier key of dict item
 * @param[in] *ref        reference to object
 */
void
set_ref_at_cur_varmap(ast_t *ast, const char *identifier, object_t *ref);
