#pragma once
#include <stdint.h>

struct object_dict;
typedef struct object_dict object_dict_t;

struct object_array;
typedef struct object_array object_array_t;

struct object;
typedef struct object object_t;

struct object_func;
typedef struct object_func object_func_t;

struct object_index;
typedef struct object_index object_index_t;

struct object_module;
typedef struct object_module object_module_t;

struct object_identifier;
typedef struct object_identifier object_identifier_t;

struct object_def_struct;
typedef struct object_def_struct object_def_struct_t;

struct object_object;
typedef struct object_object object_object_t;

struct object_chain;
typedef struct object_chain object_chain_t;

struct object_owners_method;
typedef struct object_owners_method object_owners_method_t;

struct object_type;
typedef struct object_type object_type_t;

struct object_builtin_func;
typedef struct object_builtin_func object_builtin_func_t;

struct PadNode_array;
typedef struct PadNode_array node_array_t;

struct PadNode_dict;
typedef struct PadNode_dict node_dict_t;

struct tokenizer;
typedef struct tokenizer tokenizer_t;

struct context;
typedef struct context PadCtx;

struct scope;
typedef struct scope scope_t;

struct PadAst;
typedef struct PadAst ast_t;

struct PadCcArgs;
typedef struct PadCcArgs cc_args_t;

struct PadTrvArgs;
typedef struct PadTrvArgs trv_args_t;

struct PadBuiltFuncArgs;
typedef struct PadBuiltFuncArgs builtin_func_args_t;

typedef object_t *(*builtin_func_t)(builtin_func_args_t *args);

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

