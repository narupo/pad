#pragma once

#include <stdbool.h>
#include <assert.h>

#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lib/unicode.h>
#include <pad/lib/cstring.h>
#include <pad/lib/memory.h>
#include <pad/lib/error.h>
#include <pad/lang/types.h>
#include <pad/lang/nodes.h>
#include <pad/lang/object_array.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/gc.h>
#include <pad/lang/chain_object.h>
#include <pad/lang/chain_objects.h>
#include <pad/lang/builtin/func_info_array.h>

typedef enum {
    // A nil object
    PAD_OBJ_TYPE__NIL,

    // A integer object
    // 整数オブジェクト
    // 整数の範囲は lang/types.h@PadIntObj を参照
    PAD_OBJ_TYPE__INT,

    // A float object
    // 浮動小数点数オブジェクト
    // 値の範囲は lang/types.h@PadFloatObj を参照
    PAD_OBJ_TYPE__FLOAT,

    // A boolean object
    // A boolean object has true or false
    // 真偽値オブジェクトは計算式の文脈では整数として扱われる
    // つまり true + true の結果は 2 になる
    PAD_OBJ_TYPE__BOOL,

    // A identifier object
    // 識別子オブジェクト
    // 変数などの識別子の名前を持つオブジェクト
    // このオブジェクトがスコープの変数マップに格納されることはない
    // この変数は、この変数の名前をキーに変数マップからオブジェクトを取得したり、
    // 格納したりするのに使われる
    PAD_OBJ_TYPE__IDENT,

    // A unicode object
    PAD_OBJ_TYPE__UNICODE,

    // A array object
    PAD_OBJ_TYPE__ARRAY,

    // A dictionary object
    PAD_OBJ_TYPE__DICT,

    // A struct PadObj
    // これは宣言。実体ではない
    PAD_OBJ_TYPE__DEF_STRUCT,

    // A object (instance of struct and others)
    PAD_OBJ_TYPE__OBJECT,

    // A function object
    // That has ref_suites of context of nodes in PadAST
    // Time to execute to execute this context
    PAD_OBJ_TYPE__FUNC,

    // A chain object
    // チェインオブジェクト
    // '.' | '[]' | '()' でアクセス可能なオブジェクト
    // 添字は配列で持っていて、参照時にこの添字を順に適用して実体を求める
    PAD_OBJ_TYPE__RING,

    // A module object
    // モジュールオブジェクト
    // このオブジェクトは内部にPadASTへの参照を持つ
    // このPadASTへの参照はモジュール内のオブジェクト群への参照である
    PAD_OBJ_TYPE__MODULE,

    // A owner's method object
    // array.push() や dict.pop("key") など、ドット演算子で繋げで呼び出すメソッド用のオブジェクト
    // owner にメソッドのオーナーオブジェクト、method_name にメソッド名が保存される
    PAD_OBJ_TYPE__OWNERS_METHOD,

    // A type object
    PAD_OBJ_TYPE__TYPE,

    // A builtin function
    PAD_OBJ_TYPE__BLTIN_FUNC,
} PadObjType;

/**
 * function object
 */
struct PadFuncObj {
    PadAST *ref_ast;  // function object refer this reference of ast on execute
    PadCtx *ref_context;  // reference of context
    PadObj *name;  // type == PAD_OBJ_TYPE__IDENT
    PadObj *args;  // type == PAD_OBJ_TYPE__ARRAY
    PadNodeAry *ref_suites;  // reference to suite (node tree) (DO NOT DELETE)
    PadNodeDict *ref_blocks;  // reference to blocks (build by block-statement) in function (DO NOT DELETE)
    PadObj *extends_func;  // reference to function object of extended
    bool is_met;  // is method?
};

/**
 * A module object
 */
struct PadModObj {
    char *name;  // module name
    char *program_filename;
    char *program_source;
    PadTkr *tokenizer;
    PadAST *ast;
    PadCtx *context;
    PadBltFuncInfoAry *builtin_func_infos;  // builtin functions
};

/**
 * A identifier object
 */
struct PadIdentObj {
    PadCtx *ref_context;
    PadStr *name;
};

/**
 * A chain object
 */
struct PadRingObj {
    PadObj *operand;
    PadChainObjs *chain_objs;
};

/**
 * A owner's method object
 */
struct PadOwnsMethodObj {
    PadObj *owner;
    PadStr *method_name;
};

/**
 * A struct PadObj
 */
struct PadDefStructObj {
    PadAST *ref_ast;  // reference
    PadObj *identifier;  // moved (type == PAD_OBJ_TYPE__UNICODE)
    PadAST *ast;  // moved (struct's ast (node tree))
    PadCtx *context;  // moved (struct's context)
};

/**
 * A instance of struct (and class)
 */
struct PadObjObj {
    PadAST *ref_ast;  // DO NOT DELETE
    PadAST *ref_struct_ast;  // DO NOT DELETE
    PadCtx *struct_context;  // moved
    PadObj *ref_def_obj;  // DO NOT DELETE
};

struct PadTypeObj {
    PadObjType type;
    const char *name;
};

struct PadBltFuncObj {
    const char *funcname;
};

/**
 * A abstract object
 */
struct PadObj {
    PadObjType type;  // object type
    PadGC *ref_gc;  // reference to gc (DO NOT DELETE)
    PadGCItem gc_item;  // gc item for memory management
    PadIdentObj identifier;  // value of identifier (type == PAD_OBJ_TYPE__IDENT)
    PadUni *unicode;  // value of unicode (type == PAD_OBJ_TYPE__UNICODE)
    PadObjAry *objarr;  // value of array (type == PAD_OBJ_TYPE__ARRAY)
    PadObjDict *objdict;  // value of dict (type == PAD_OBJ_TYPE__DICT)
    PadIntObj lvalue;  // value of integer (type == PAD_OBJ_TYPE__INT)
    PadFloatObj float_value;  // value of float (type == PAD_OBJ_TYPE__FLOAT)
    bool boolean;  // value of boolean (type == PAD_OBJ_TYPE__BOOL)
    PadFuncObj func;  // structure of function (type == PAD_OBJ_TYPE__FUNC)
    PadDefStructObj def_struct;  // structure of pad's structure (type == PAD_OBJ_TYPE__DEF_STRUCT)
    PadObjObj object;  // structure of object (type == PAD_OBJ_TYPE__INSTANCE)
    PadModObj module;  // structure of module (type == PAD_OBJ_TYPE__MODULE)
    PadRingObj chain;  // structure of chain (type == PAD_OBJ_TYPE__RING)
    PadOwnsMethodObj owners_method;  // structure of owners_method (type == PAD_OBJ_TYPE__OWNERS_METHOD)
    PadTypeObj type_obj;  // structure of type (type == PAD_OBJ_TYPE__TYPE)
    PadBltFuncObj builtin_func;  // structure of builtin func (type == PAD_OBJ_TYPE__BLTIN_FUNC)
};

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to PadObj
 */
void
PadObj_Del(PadObj *self);

/**
 * construct PadObj
 *
 * @param[in] *ref_gc reference to PadGC (DO NOT DELETE)
 * @param[in] type number of object type
 *
 * @return success to pointer to PadObj (dynamic allocate memory)
 * @return failed to NULL
 */
PadObj *
PadObj_New(PadGC *ref_gc, PadObjType type);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
PadObj *
PadObj_DeepCopy(const PadObj *other);

/**
 * shallow copy
 *
 * @param[in] *other
 *
 * @return
 */
PadObj *
PadObj_ShallowCopy(const PadObj *other);

/**
 * construct nil object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewNil(PadGC *ref_gc);

/**
 * construct false of boolean object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewFalse(PadGC *ref_gc);

/**
 * construct true of boolean object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewTrue(PadGC *ref_gc);

/**
 * construct boolean object by value
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 * @param[in] boolean value of boolean
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewBool(PadGC *ref_gc, bool boolean);

/**
 * construct identifier object by C string
 * if failed to allocate memory then exit from process
 *
 * @param[in]      *ref_gc      reference to PadGC (do not delete)
 * @param[in|out]  *ref_ast     reference to PadAST current context (do not delete)
 * @param [in|out] *ref_context reference to PadCtx
 * @param[in]      *identifier  C strings of identifier
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewCIdent(
    PadGC *ref_gc,
    PadCtx *ref_context,
    const char *identifier
);

/**
 * construct identifier object by PadStr
 * if failed to allocate memory then exit from process
 *
 * @param[in]     *ref_gc          reference to PadGC (do not delete)
 * @param[in|out] *ref_ast         reference to PadAST current context (do not delete)
 * @param [in|out] *ref_context    reference to PadCtx
 * @param[in]     *move_identifier pointer to PadStr (with move semantics)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewIdent(
    PadGC *ref_gc,
    PadCtx *ref_context,
    PadStr *move_identifier
);

/**
 * construct PadUni object by C strings
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 * @param[in] *str    pointer to C strings
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewUnicodeCStr(PadGC *ref_gc, const char *str);

/**
 * construct PadUni object by PadUni
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc   reference to PadGC (do not delete)
 * @param[in] *move_str pointer to PadStr (with move semantics)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewUnicode(PadGC *ref_gc, PadUni *move_unicode);

/**
 * construct integer object by value
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 * @param[in] lvalue  value of integer
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewInt(PadGC *ref_gc, PadIntObj lvalue);

/**
 * construct float object by value
 * if failed to allocate memory then exit from process
 * 
 * @param[in] *ref_gc reference to PadGC (do not delete)
 * @param[in] value  value of PadFloatObj
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewFloat(PadGC *ref_gc, PadFloatObj value);

/**
 * construct array object by PadObjAry
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc      reference to PadGC (do not delete)
 * @param[in] *move_objarr pointer to PadObjAry (with move semantics)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewAry(PadGC *ref_gc, PadObjAry *move_objarr);

/**
 * construct PadDictionary object by PadObjDict
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc       reference to PadGC (do not delete)
 * @param[in] *move_objdict pointer to PadObjDict (with move semantics)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewDict(PadGC *ref_gc, PadObjDict *move_objdict);

/**
 * construct function object by parameters
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc       reference to PadGC (do not delete)
 * @param[in] *ref_ast      reference to PadAST (do not delete). The function object refer this context
 * @param[in] *move_name    pointer to identifier object for function name (with move semantics)
 * @param[in] *move_args    pointer to array object for function arguments (with move semantics)
 * @param[in] *ref_suites   reference to nodes of function content (do not delete)
 * @param[in] *ref_blocks   reference to dict nodes of function blocks (do not delete)
 * @param[in] *extends_func reference to function of extended
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewFunc(
    PadGC *ref_gc,
    PadAST *ref_ast,
    PadCtx *ref_context,
    PadObj *move_name,
    PadObj *move_args,
    PadNodeAry *ref_suites,
    PadNodeDict *ref_blocks,
    PadObj *extends_func,
    bool is_met
);

/**
 * construct ring object by parameters
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc          reference to PadGC (do not delete)
 * @param[in] *move_operand    pointer to PadObj (move semantics)
 * @param[in] *move_chain_objs pointer to PadChainObjs (move semantics)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewRing(PadGC *ref_gc, PadObj *move_operand, PadChainObjs *move_chain_objs);

/**
 * construct module object (default constructor)
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc reference to PadGC (do not delete)
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewMod(PadGC *ref_gc);

/**
 * construct def-struct-object 
 * if failed to allocate memory then exit from process
 *
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewDefStruct(
    PadGC *ref_gc,
    PadObj *move_idn,
    PadAST *move_ast,
    PadCtx *move_context
);

/**
 * construct PadObj object
 * if failed to allocate memory then exit from process
 * 
 * @return success to pointer to PadObj (new object)
 * @return failed to NULL
 */
PadObj *
PadObj_NewObj(
    PadGC *ref_gc,
    PadAST *ref_ast,
    PadCtx *move_context,
    PadObj *ref_def_obj
);

/**
 * construct module object by parameters
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc              reference to PadGC (do not delete)
 * @param[in] *name                pointer to C strings for module name
 * @param[in] *program_filename    file name
 * @param[in] *move_program_source program source
 * @param[in] *move_tkr            pointer to PadTkr (with move semantics)
 * @param[in] *move_ast            pointer to PadAST (with move semantics)
 * @param[in] *move_context        context
 * @param[in] *func_infos          array of functions
 *
 * @return pointer to PadObj
 */
PadObj *
PadObj_NewModBy(
    PadGC *ref_gc,
    const char *name,
    const char *program_filename,
    char *move_program_source,
    PadTkr *move_tkr,
    PadAST *move_ast,
    PadCtx *move_context,
    PadBltFuncInfoAry *func_infos
);

/**
 * construct owner's method object
 * if failed to allocate memory then exit from process
 *
 * @param[in] *ref_gc
 * @param[in] *owner       owner object
 * @param[in] *method_name method name
 *
 * @return pointer to PadObj
 */
PadObj *
PadObj_NewOwnsMethod(PadGC *ref_gc, PadObj *owner, PadStr *move_method_name);

PadObj *
PadObj_NewType(PadGC *ref_gc, PadObjType type);

PadObj *
PadObj_NewBltFunc(PadGC *ref_gc, const char *funcname);

/**
 * object to PadStr
 *
 * @param[in] *self
 *
 * @return pointer to PadStr (new PadStr)
 * @return failed to NULL
 */
PadStr *
PadObj_ToStr(const PadObj *self);

/**
 * various object convert to array object
 *
 * @param[in] *self
 *
 * @return success to pointer to array object (PAD_OBJ_TYPE__ARRAY)
 * @return failed to NULL
 */
PadObj *
PadObj_ToAry(const PadObj *self);

/**
 * increment reference count of object
 *
 * @param[in] *self
 */
void
PadObj_IncRef(PadObj *self);

/**
 * decrement reference count of object
 *
 * @param[in] *self
 */
void
PadObj_DecRef(PadObj *self);

/**
 * get reference of PadGCItem in object
 *
 * @param[in] *self
 *
 * @return reference to PadGCItem (do not delete)
 * @return failed to NULL
 */
PadGCItem *
PadObj_GetGcItem(PadObj *self);

/**
 * dump PadObj at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadObj_Dump(const PadObj *self, FILE *fout);

/**
 * object type to string
 *
 * @param[in] *self
 *
 * @return pointer to new PadStr
 */
PadStr *
PadObj_TypeToStr(const PadObj *self);

/**
 * get identifier name
 *
 * @param[in] *self
 *
 * @return pointer to strings in identifier
 */
const char *
PadObj_GetcIdentName(const PadObj *self);

/**
 * get built-in function name
 * 
 * @param[in] *self
 * 
 * @return 
 */
const char *
PadObj_GetcBltFuncName(const PadObj *self);

/**
 * get def-struct identifier value
 *
 * @param[in] *self
 *
 * @return pointer to strings in identifier
 */
const char *
PadObj_GetcDefStructIdentName(const PadObj *self);

PadCtx *
PadObj_GetIdentRefCtx(const PadObj *self);

/**
 * get chain objects in chain object (type == PAD_OBJ_TYPE__RING)
 *
 * @param[in] *self
 *
 * @return pointer to PadChainObjs
 */
PadChainObjs *
PadObj_GetChainObjs(PadObj *self);

/**
 * get chain objects in chain object read-only (type == PAD_OBJ_TYPE__RING)
 *
 * @param[in] *self
 *
 * @return pointer to PadChainObjs
 */
const PadChainObjs *
PadObj_GetcChainObjs(const PadObj *self);

/**
 * get operand in chain object
 *
 * @param[in] *self
 *
 * @return pointer to PadObj
 */
PadObj *
PadObj_GetChainOperand(PadObj *self);

/**
 * get operand in chain object read-only
 *
 * @param[in] *self
 *
 * @return pointer to PadObj
 */
const PadObj *
PadObj_GetcChainOperand(const PadObj *self);

/**
 * get function name of func-object
 *
 * @param[in] *self
 *
 * @return pointer to strings of function name
 */
const char *
PadObj_GetcFuncName(const PadObj *self);

/**
 * get array of array-object
 *
 * @param[in] *self
 *
 * @return pointer to array
 */
PadObjAry *
PadObj_GetAry(PadObj *self);

/**
 * get array of array-object read-only
 *
 * @param[in] *self
 *
 * @return pointer to array
 */
const PadObjAry *
PadObj_GetcAry(const PadObj *self);

/**
 * get dict of dict-object
 *
 * @param[in] *self
 *
 * @return pointer to dict
 */
PadObjDict *
PadObj_GetDict(PadObj *self);

/**
 * get dict of dict-object read-only
 *
 * @param[in] *self
 *
 * @return pointer to dict
 */
const PadObjDict *
PadObj_GetcDict(const PadObj *self);

/**
 * get unicode in PadObj read-only
 *
 * @param[in] *self
 *
 * @return pointer to PadUni
 */
const PadUni *
PadObj_GetcUnicode(const PadObj *self);

/**
 * get unicode in PadObj 
 *
 * @param[in] *self
 *
 * @return pointer to PadUni
 */
PadUni *
PadObj_GetUnicode(PadObj *self);

/**
 * get builtin function informations
 *
 * @param[in] *self
 *
 * @return pointer to PadBltFuncInfo
 */
PadBltFuncInfoAry *
PadObj_GetModBltFuncInfos(const PadObj *self);

/**
 * get method name of owner's method object read-only
 *
 * @param[in] *self
 *
 * @return pointer to PadStr
 */
const PadStr *
PadObj_GetcOwnsMethodName(const PadObj *self);

/**
 * get owner of owner's method object
 *
 * @param[in] *self
 *
 * @return pointer to PadObj
 */
PadObj *
PadObj_GetOwnsMethodOwn(PadObj *self);

/**
 * get module name
 *
 * @param[in] *self
 *
 * @return pointer to C strings
 */
const char *
PadObj_GetcModName(const PadObj *self);

PadGC *
PadObj_GetGc(PadObj *self);

PadGC *
PadObj_SetGC(PadObj *self, PadGC *ref_gc);
