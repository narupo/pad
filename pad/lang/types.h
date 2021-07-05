#pragma once
#include <stdint.h>

struct PadObj_dict;
typedef struct PadObj_dict PadObjDict;

struct PadObjAry;
typedef struct PadObjAry PadObjAry;

struct PadObj;
typedef struct PadObj PadObj;

struct PadFuncObj;
typedef struct PadFuncObj object_func_t;

struct PadObj_index;
typedef struct PadObj_index object_index_t;

struct PadModObj;
typedef struct PadModObj object_module_t;

struct PadIdentObj;
typedef struct PadIdentObj object_identifier_t;

struct PadDefStructObj;
typedef struct PadDefStructObj object_def_struct_t;

struct PadObjObj;
typedef struct PadObjObj object_PadObj;

struct PadChainObj;
typedef struct PadChainObj object_chain_t;

struct PadOwnsMethodObj;
typedef struct PadOwnsMethodObj object_owners_method_t;

struct PadTypeObj;
typedef struct PadTypeObj PadObjype_t;

struct PadBltFuncObj;
typedef struct PadBltFuncObj object_builtin_func_t;

struct PadNode_array;
typedef struct PadNode_array PadNodeAry;

struct PadNode_dict;
typedef struct PadNode_dict PadNodeDict;

struct PadTkr;
typedef struct PadTkr PadTkr;

struct context;
typedef struct context PadCtx;

struct scope;
typedef struct scope PadScope;

struct PadAst;
typedef struct PadAst PadAST;

struct PadCcArgs;
typedef struct PadCcArgs cc_args_t;

struct PadTrvArgs;
typedef struct PadTrvArgs trv_args_t;

struct PadBuiltFuncArgs;
typedef struct PadBuiltFuncArgs builtin_func_args_t;

typedef PadObj *(*builtin_func_t)(builtin_func_args_t *args);

typedef struct PadBuiltFuncInfo builtin_func_info_t;

struct gc;
typedef struct gc PadGc;

struct PadGcItem;
typedef struct PadGcItem PadGcItem;

typedef long objint_t;
typedef double objfloat_t;

// number of depth type for function recursion
typedef int32_t depth_t;

struct chain_node;
typedef struct chain_node PadChainNode;

struct chain_nodes;
typedef struct chain_nodes PadChainNodes;

struct chain_object;
typedef struct chain_object PadChainObj;

struct chain_objects;
typedef struct chain_objects PadChainObjs;

struct PadNode;
typedef struct PadNode PadNode;

struct kit;
typedef struct kit PadKit;

