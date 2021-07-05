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
} PadIndexValue;

/**
 * The abstract arguments of compiler functions
 */
struct PadCcArgs {
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
    PadFuncDefNode *func_def;
};

/**
 * The abstract arguments of traverser functions
 */
struct PadTrvArgs {
    // references of objects of owners
    // traverser refer this owners on context
    // references. do not delete objects in array
    PadObjAry *ref_owners;

    // reference of node
    // traverser parse this node and return result as PadObj
    // reference. do not delete
    PadNode *ref_node;

    // node of compare operator
    PadCompOpNode *comp_op_node;

    // node add-sub operator
    PadAddSubOpNode *add_sub_op_node;

    // node mul-div operator
    PadMulDivOpNode *mul_div_op_node;

    // node augassign operator
    PadAugassignNode *augassign_op_node;

    // number of depth of function
    depth_t depth;

    // left hand and right hand objects
    // these objects refer from calc functions
    PadObj *lhs_obj;
    PadObj *rhs_obj;

    // reference of object
    PadObj *ref_obj;

    // read-only reference of object
    const PadObj *cref_obj;

    // actual arguments
    PadObj *ref_args_obj;

    // function name
    const char *funcname;

    // identifier name
    const char *identifier;

    // a index value object
    PadIndexValue index_value;

    // a callback function
    PadObj * (*callback)(PadAST *ast, struct PadTrvArgs *targs);

    // if do not refer chain object on context then store true else store false
    // this flag refer in trv_chain function
    bool do_not_refer_chain;

    // if current context is in function block then store pointer
    // of function object to this func_obj variable
    PadObj *func_obj;
};

/**
 * The abstract arguments for builtin functions
 */
struct PadBuiltFuncArgs {
    PadAST *ref_ast;  // the ast of current context
    const PadNode *ref_node;  // reference of node for errors
    PadObj *ref_args;  // the arguments object of builtin functions
    PadObjAry *ref_owners;  // reference to owners of array
};

