#pragma once

#include <stdbool.h>
#include <assert.h>

#include <pad/lib/string.h>
#include <pad/lib/unicode.h>
#include <pad/lib/cstring.h>
#include <pad/lib/memory.h>
#include <pad/lib/error.h>
#include <pad/lang/types.h>
#include <pad/lang/nodes.h>
#include <pad/lang/object_array.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/gc.h>
#include <pad/lang/builtin/function.h>
#include <pad/lang/chain_object.h>
#include <pad/lang/chain_objects.h>

typedef enum {
    // A nil object
    OBJ_TYPE_NIL,

    // A integer object
    // 整数オブジェクト
    // 整数の範囲は lang/types.h@objint_t を参照
    OBJ_TYPE_INT,

    // A boolean object
    // A boolean object has true or false
    // 真偽値オブジェクトは計算式の文脈では整数として扱われる
    // つまり true + true の結果は 2 になる
    OBJ_TYPE_BOOL,

    // A identifier object
    // 識別子オブジェクト
    // 変数などの識別子の名前を持つオブジェクト
    // このオブジェクトがスコープの変数マップに格納されることはない
    // この変数は、この変数の名前をキーに変数マップからオブジェクトを取得したり、
    // 格納したりするのに使われる
    OBJ_TYPE_IDENTIFIER,

    // A unicode object
    OBJ_TYPE_UNICODE,

    // A array object
    OBJ_TYPE_ARRAY,

    // A dictionary object
    OBJ_TYPE_DICT,

    // A struct object
    OBJ_TYPE_DEF_STRUCT,

    // A object (instance of struct and others)
    OBJ_TYPE_OBJECT,

    // A function object
    // That has ref_suites of context of nodes in ast_t
    // Time to execute to execute this context
    OBJ_TYPE_FUNC,

    // A chain object
    // チェインオブジェクト
    // '.' | '[]' | '()' でアクセス可能なオブジェクト
    // 添字は配列で持っていて、参照時にこの添字を順に適用して実体を求める
    OBJ_TYPE_CHAIN,

    // A module object
    // モジュールオブジェクト
    // このオブジェクトは内部にast_tへの参照を持つ
    // このast_tへの参照はモジュール内のオブジェクト群への参照である
    OBJ_TYPE_MODULE,

    // A owner's method object
    // array.push() や dict.pop("key") など、ドット演算子で繋げで呼び出すメソッド用のオブジェクト
    // owner にメソッドのオーナーオブジェクト、method_name にメソッド名が保存される
    OBJ_TYPE_OWNERS_METHOD,
} obj_type_t;

/**
 * function object
 */
struct object_func {
    ast_t *ref_ast;  // function object refer this reference of ast on execute
    object_t *name;  // type == OBJ_TYPE_IDENTIFIER
    object_t *args;  // type == OBJ_TYPE_ARRAY
    node_array_t *ref_suites;  // reference to suite (node tree) (DO NOT DELETE)
    node_dict_t *ref_blocks;  // reference to blocks (build by block-statement) in function (DO NOT DELETE)
    object_t *extends_func;  // reference to function object of extended
};

/**
 * A module object
 */
struct object_module {
    string_t *name;  // module name
    tokenizer_t *tokenizer;
    ast_t *ast;
    builtin_func_info_t *builtin_func_infos;  // builtin functions
};

/**
 * A identifier object
 */
struct object_identifier {
    ast_t *ref_ast;
    string_t *name;
};

/**
 * A chain object
 */
struct object_chain {
    object_t *operand;
    chain_objects_t *chain_objs;
};

/**
 * A owner's method object
 */
struct object_owners_method {
    object_t *owner;
    string_t *method_name;
};

/**
 * A struct object
 */
struct object_def_struct {
    ast_t *ref_ast;  // reference
    object_t *identifier;  // moved (type == OBJ_TYPE_UNICODE)
    ast_t *ast;  // moved (struct's ast (node tree))
    context_t *context;  // moved (struct's context)
};

/**
 * A instance of struct (and class)
 */
struct object_object {
    ast_t *ref_ast;  // DO NOT DELETE
    ast_t *ref_struct_ast;  // DO NOT DELETE
    context_t *struct_context;  // moved
};

/**
 * A abstract object
 */
struct object {
    obj_type_t type;  // object type
    gc_t *ref_gc;  // reference to gc (DO NOT DELETE)
    gc_item_t gc_item;  // gc item for memory management
    object_identifier_t identifier;  // value of identifier (type == OBJ_TYPE_IDENTIFIER)
    unicode_t *unicode;  // value of unicode (type == OBJ_TYPE_UNICODE)
    object_array_t *objarr;  // value of array (type == OBJ_TYPE_ARRAY)
    object_dict_t *objdict;  // value of dict (type == OBJ_TYPE_DICT)
    objint_t lvalue;  // value of integer (type == OBJ_TYPE_INT)
    bool boolean;  // value of boolean (type == OBJ_TYPE_BOOL)
    object_func_t func;  // structure of function (type == OBJ_TYPE_FUNC)
    object_def_struct_t def_struct;  // structure of pad's structure (type == OBJ_TYPE_DEF_STRUCT)
    object_object_t object;  // structure of object (type == OBJ_TYPE_INSTANCE)
    object_module_t module;  // structure of module (type == OBJ_TYPE_MODULE)
    object_chain_t chain;  // structure of chain (type == OBJ_TYPE_CHAIN)
    object_owners_method_t owners_method;  // structure of owners_method (type == OBJ_TYPE_OWNERS_METHOD)
};

/**
 * destruct object
 *
 * @param[in] *self pointer to object_t
 */
void
obj_del(object_t *self);

/**
 * construct object
 *
 * @param[in] *ref_gc reference to gc_t (DO NOT DELETE)
 * @param[in] type number of object type
 *
 * @return success to pointer to object_t (dynamic allocate memory)
 * @return failed to NULL
 */
object_t *
obj_new(gc_t *ref_gc, obj_type_t type);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
object_t *
obj_deep_copy(const object_t *other);

/**
 * construct nil object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_nil(gc_t *ref_gc);

/**
 * construct false of boolean object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_false(gc_t *ref_gc);

/**
 * construct true of boolean object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_true(gc_t *ref_gc);

/**
 * construct boolean object by value
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 * @param[in] boolean value of boolean
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_bool(gc_t *ref_gc, bool boolean);

/**
 * construct identifier object by C string
 * if failed to allocate memory then exit from process
 *
 * @param[in]     *ref_gc     reference to gc_t (do not delete)
 * @param[in|out] *ref_ast    reference to ast_t current context (do not delete)
 * @param[in]     *identifier C strings of identifier
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_cidentifier(gc_t *ref_gc, ast_t *ref_ast, const char *identifier);

/**
 * construct identifier object by string_t
 * if failed to allocate memory then exit from process
 *
 * @param[in]     *ref_gc          reference to gc_t (do not delete)
 * @param[in|out] *ref_ast         reference to ast_t current context (do not delete)
 * @param[in]     *move_identifier pointer to string_t (with move semantics)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_identifier(gc_t *ref_gc, ast_t *ref_ast, string_t *move_identifier);

/**
 * construct unicode object by C strings
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 * @param[in] *str    pointer to C strings
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_unicode_cstr(gc_t *ref_gc, const char *str);

/**
 * construct unicode object by unicode_t
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc   reference to gc_t (do not delete)
 * @param[in] *move_str pointer to string_t (with move semantics)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_unicode(gc_t *ref_gc, unicode_t *move_unicode);

/**
 * construct integer object by value
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 * @param[in] lvalue  value of integer
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_int(gc_t *ref_gc, objint_t lvalue);

/**
 * construct array object by object_array_t
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc      reference to gc_t (do not delete)
 * @param[in] *move_objarr pointer to object_array_t (with move semantics)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_array(gc_t *ref_gc, object_array_t *move_objarr);

/**
 * construct dictionary object by object_dict_t
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc       reference to gc_t (do not delete)
 * @param[in] *move_objdict pointer to object_dict_t (with move semantics)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_dict(gc_t *ref_gc, object_dict_t *move_objdict);

/**
 * construct function object by parameters
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc       reference to gc_t (do not delete)
 * @param[in] *ref_ast      reference to ast_t (do not delete). The function object refer this context
 * @param[in] *move_name    pointer to identifier object for function name (with move semantics)
 * @param[in] *move_args    pointer to array object for function arguments (with move semantics)
 * @param[in] *ref_suites   reference to nodes of function content (do not delete)
 * @param[in] *ref_blocks   reference to dict nodes of function blocks (do not delete)
 * @param[in] *extends_func reference to function of extended
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_func(
    gc_t *ref_gc,
    ast_t *ref_ast,
    object_t *move_name,
    object_t *move_args,
    node_array_t *ref_suites,
    node_dict_t *ref_blocks,
    object_t *extends_func
);

/**
 * construct chain object by parameters
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc          reference to gc_t (do not delete)
 * @param[in] *move_operand    pointer to object_t (move semantics)
 * @param[in] *move_chain_objs pointer to chain_objects_t (move semantics)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_chain(gc_t *ref_gc, object_t *move_operand, chain_objects_t *move_chain_objs);

/**
 * construct module object (default constructor)
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_module(gc_t *ref_gc);

/**
 * construct def-struct-object 
 * if failed to allocate memory then exit from process
 *
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_def_struct(
    gc_t *ref_gc,
    object_t *move_idn,
    ast_t *move_ast,
    context_t *move_context
);

/**
 * construct object object
 * if failed to allocate memory then exit from process
 * 
 * @return success to pointer to object_t (new object)
 * @return failed to NULL
 */
object_t *
obj_new_object(
    gc_t *ref_gc,
    ast_t *ref_ast,
    context_t *move_struct_context
);

/**
 * construct module object by parameters
 * if failed to allocate memory then exit from process
 *
 *
 * @param[in] *ref_gc     reference to gc_t (do not delete)
 * @param[in] *name       pointer to C strings for module name
 * @param[in] *move_tkr   pointer to tokenizer_t (with move semantics)
 * @param[in] *move_ast   pointer to ast_t (with move semantics)
 * @param[in] *func_infos array of functions
 *
 * @return pointer to object_t
 */
object_t *
obj_new_module_by(
    gc_t *ref_gc,
    const char *name,
    tokenizer_t *move_tkr,
    ast_t *move_ast,
    builtin_func_info_t *func_infos
);

/**
 * construct owner's method object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc
 * @param[in] *owner       owner object
 * @param[in] *method_name method name
 *
 * @return pointer to object_t
 */
object_t *
obj_new_owners_method(gc_t *ref_gc, object_t *owner, string_t *move_method_name);

/**
 * object to string_t
 *
 * @param[in] *self
 *
 * @return pointer to string_t (new string_t)
 * @return failed to NULL
 */
string_t *
obj_to_str(const object_t *self);

/**
 * various object convert to array object
 *
 * @param[in] *self
 *
 * @return success to pointer to array object (OBJ_TYPE_ARRAY)
 * @return failed to NULL
 */
object_t *
obj_to_array(const object_t *self);

/**
 * increment reference count of object
 *
 * @param[in] *self
 */
void
obj_inc_ref(object_t *self);

/**
 * decrement reference count of object
 *
 * @param[in] *self
 */
void
obj_dec_ref(object_t *self);

/**
 * get reference of gc_item_t in object
 *
 * @param[in] *self
 *
 * @return reference to gc_item_t (do not delete)
 * @return failed to NULL
 */
gc_item_t *
obj_get_gc_item(object_t *self);

/**
 * dump object_t at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
obj_dump(const object_t *self, FILE *fout);

/**
 * object type to string
 *
 * @param[in] *self
 *
 * @return pointer to new string_t
 */
string_t *
obj_type_to_str(const object_t *self);

/**
 * get identifier name
 *
 * @param[in] *self
 *
 * @return pointer to strings in identifier
 */
const char *
obj_getc_idn_name(const object_t *self);

/**
 * get def-struct identifier value
 *
 * @param[in] *self
 *
 * @return pointer to strings in identifier
 */
const char *
obj_getc_def_struct_idn_name(const object_t *self);

/**
 * get reference of ast in identifier object
 *
 * @param[in] *self
 *
 * @return reference to ast_t (do not delete)
 */
ast_t *
obj_get_idn_ref_ast(const object_t *self);

/**
 * get chain objects in chain object (type == OBJ_TYPE_CHAIN)
 *
 * @param[in] *self
 *
 * @return pointer to chain_objects_t
 */
chain_objects_t *
obj_get_chain_objs(object_t *self);

/**
 * get chain objects in chain object read-only (type == OBJ_TYPE_CHAIN)
 *
 * @param[in] *self
 *
 * @return pointer to chain_objects_t
 */
const chain_objects_t *
obj_getc_chain_objs(const object_t *self);

/**
 * get operand in chain object
 *
 * @param[in] *self
 *
 * @return pointer to object_t
 */
object_t *
obj_get_chain_operand(object_t *self);

/**
 * get operand in chain object read-only
 *
 * @param[in] *self
 *
 * @return pointer to object_t
 */
const object_t *
obj_getc_chain_operand(const object_t *self);

/**
 * get function name of func-object
 *
 * @param[in] *self
 *
 * @return pointer to strings of function name
 */
const char *
obj_getc_func_name(const object_t *self);

/**
 * get array of array-object
 *
 * @param[in] *self
 *
 * @return pointer to array
 */
object_array_t *
obj_get_array(object_t *self);

/**
 * get array of array-object read-only
 *
 * @param[in] *self
 *
 * @return pointer to array
 */
const object_array_t *
obj_getc_array(const object_t *self);

/**
 * get dict of dict-object
 *
 * @param[in] *self
 *
 * @return pointer to dict
 */
object_dict_t *
obj_get_dict(object_t *self);

/**
 * get dict of dict-object read-only
 *
 * @param[in] *self
 *
 * @return pointer to dict
 */
const object_dict_t *
obj_getc_dict(const object_t *self);

/**
 * get unicode in object_t read-only
 *
 * @param[in] *self
 *
 * @return pointer to unicode_t
 */
const unicode_t *
obj_getc_unicode(const object_t *self);

/**
 * get unicode in object_t 
 *
 * @param[in] *self
 *
 * @return pointer to unicode_t
 */
unicode_t *
obj_get_unicode(object_t *self);

/**
 * get builtin function informations
 *
 * @param[in] *self
 *
 * @return pointer to builtin_func_info_t
 */
builtin_func_info_t *
obj_get_module_builtin_func_infos(const object_t *self);

/**
 * get method name of owner's method object read-only
 *
 * @param[in] *self
 *
 * @return pointer to string_t
 */
const string_t *
obj_getc_owners_method_name(const object_t *self);

/**
 * get owner of owner's method object
 *
 * @param[in] *self
 *
 * @return pointer to object_t
 */
object_t *
obj_get_owners_method_owner(object_t *self);

/**
 * get module name
 *
 * @param[in] *self
 *
 * @return pointer to C strings
 */
const string_t *
obj_getc_mod_name(const object_t *self);
