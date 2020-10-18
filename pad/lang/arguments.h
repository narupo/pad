#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/object.h>
#include <pad/lang/object_array.h>
#include <pad/lang/nodes.h>

/**
 * A index value object
 */
typedef struct {
    char type;  // 's' ... string, 'i' ... integer
    const char *skey;  // index value of string
    long ikey;  // index value of int
} index_value_t;

/**
 * The abstract arguments of compiler functions
 */
struct cc_args {
    // number of depth of recursion
    depth_t depth;

    // if current context is in loop statement then store true else store false
    bool is_in_loop;

    // if current context is in function the store true else store false
    bool is_in_func;

    // if statement type
    // 0 ... if, 1 ... elif
    int if_stmt_type;

    // if context is in function to enable this pointer
    node_func_def_t *func_def;
};

/**
 * The abstract arguments of traverser functions
 */
struct trv_args {
    // references of objects of owners
    // traverser refer this owners on context
    // references. do not delete objects in array
    object_array_t *ref_owners;

    // reference of node
    // traverser parse this node and return result as object_t
    // reference. do not delete
    node_t *ref_node;

    // node of compare operator
    node_comp_op_t *comp_op_node;

    // node add-sub operator
    node_add_sub_op_t *add_sub_op_node;

    // node mul-div operator
    node_mul_div_op_t *mul_div_op_node;

    // node augassign operator
    node_augassign_t *augassign_op_node;

    // number of depth of function
    depth_t depth;

    // left hand and right hand objects
    // these objects refer from calc functions
    object_t *lhs_obj;
    object_t *rhs_obj;

    // reference of object
    object_t *ref_obj;

    // read-only reference of object
    const object_t *cref_obj;

    // actual arguments
    object_t *ref_args_obj;

    // function name
    const char *funcname;

    // identifier name
    const char *identifier;

    // a index value object
    index_value_t index_value;

    // a callback function
    object_t * (*callback)(ast_t *ast, struct trv_args *targs);

    // if do not refer chain object on context then store true else store false
    // this flag refer in trv_chain function
    bool do_not_refer_chain;

    // if current context is in function block then store pointer
    // of function object to this func_obj variable
    object_t *func_obj;
};

/**
 * The abstract arguments for builtin functions
 */
struct builtin_func_args {
    ast_t *ref_ast;  // the ast of current context
    object_t *ref_args;  // the arguments object of builtin functions
    object_array_t *ref_owners;  // reference to owners of array
};