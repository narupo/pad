#pragma once

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

struct object_reserv;
typedef struct object_reserv object_reserv_t;

struct object_identifier;
typedef struct object_identifier object_identifier_t;

struct node_array;
typedef struct node_array node_array_t;

struct tokenizer;
typedef struct tokenizer tokenizer_t;

struct context;
typedef struct context context_t;

struct scope;
typedef struct scope scope_t;

struct ast;
typedef struct ast ast_t;

struct trv_args;
typedef struct trv_args trv_args_t;

struct  builtin_func_args;
typedef struct  builtin_func_args builtin_func_args_t;

struct builtin_func_args;
typedef struct builtin_func_args builtin_func_args_t;

typedef object_t *(*builtin_func_t)(builtin_func_args_t *args);

typedef struct builtin_func_info builtin_func_info_t;

struct gc;
typedef struct gc gc_t;

struct gc_item;
typedef struct gc_item gc_item_t;

typedef long objint_t;

// number of depth type for function recursion
typedef int32_t depth_t;
