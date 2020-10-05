#include <pad/lang/nodes.h>

/*******
* node *
*******/

void
node_del(node_t *self) {
    if (!self) {
        return;
    }

    free(self->real);
    free(self);
}

node_t *
node_new(node_type_t type, void *real) {
    node_t *self = mem_ecalloc(1, sizeof(*self));
    self->type = type;
    self->real = real;
    return self;
}

node_t *
node_deep_copy(const node_t *other) {
#define declare(T, name) T *name = mem_ecalloc(1, sizeof(*name))

#define copy_node_array(dst, src, member) \
    dst->member = nodearr_new(); \
    for (int32_t i = 0; i < nodearr_len(src->member); ++i) { \
        node_t *node = nodearr_get(src->member, i); \
        node = node_deep_copy(node); \
        nodearr_moveb(dst->member, node); \
    } \

#define copy_node_dict(dst, src, member) \
    dst->member = nodedict_new(); \
    for (int32_t i = 0; i < nodedict_len(src->member); ++i) { \
        const node_dict_item_t *item = nodedict_getc_index(src->member, i); \
        assert(item); \
        node_t *node = node_deep_copy(item->value); \
        nodedict_move(dst->member, item->key, mem_move(node)); \
    } \

    if (!other) {
        return NULL;
    }

    declare(node_t, self);
    self->type = other->type;

    switch (other->type) {
    case NODE_TYPE_INVALID:
        break;
    case NODE_TYPE_PROGRAM: {
        declare(node_program_t, dst);
        node_program_t *src = other->real;
        dst->blocks = node_deep_copy(src->blocks);
        self->real = dst;
    } break;
    case NODE_TYPE_BLOCKS: {
        declare(node_blocks_t, dst);
        node_blocks_t *src = other->real;
        dst->code_block = node_deep_copy(src->code_block);
        dst->ref_block = node_deep_copy(src->ref_block);
        dst->text_block = node_deep_copy(src->text_block);
        dst->blocks = node_deep_copy(src->blocks);
        self->real = dst;
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        declare(node_code_block_t, dst);
        node_code_block_t *src = other->real;
        dst->elems = node_deep_copy(src->elems);
        self->real = dst;
    } break;
    case NODE_TYPE_REF_BLOCK: {
        declare(node_ref_block_t, dst);
        node_ref_block_t *src = other->real;
        dst->formula = node_deep_copy(src->formula);
        self->real = dst;
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        declare(node_text_block_t, dst);
        node_text_block_t *src = other->real;
        dst->text = cstr_edup(src->text);
        self->real = dst;
    } break;
    case NODE_TYPE_ELEMS: {
        declare(node_elems_t, dst);
        node_elems_t *src = other->real;
        dst->def = node_deep_copy(src->def);
        dst->stmt = node_deep_copy(src->stmt);
        dst->formula = node_deep_copy(src->formula);
        dst->elems = node_deep_copy(src->elems);
        self->real = dst;
    } break;
    case NODE_TYPE_STMT: {
        declare(node_stmt_t, dst);
        node_stmt_t *src = other->real;
        dst->import_stmt = node_deep_copy(src->import_stmt);
        dst->if_stmt = node_deep_copy(src->if_stmt);
        dst->for_stmt = node_deep_copy(src->for_stmt);
        dst->break_stmt = node_deep_copy(src->break_stmt);
        dst->continue_stmt = node_deep_copy(src->continue_stmt);
        dst->return_stmt = node_deep_copy(src->return_stmt);
        dst->block_stmt = node_deep_copy(src->block_stmt);
        dst->inject_stmt = node_deep_copy(src->inject_stmt);
        self->real = dst;
    } break;
    case NODE_TYPE_IMPORT_STMT: {
        declare(node_import_stmt_t, dst);
        node_import_stmt_t *src = other->real;
        dst->import_as_stmt = node_deep_copy(src->import_as_stmt);
        dst->from_import_stmt = node_deep_copy(src->from_import_stmt);
        self->real = dst;
    } break;
    case NODE_TYPE_IMPORT_AS_STMT: {
        declare(node_import_as_stmt_t, dst);
        node_import_as_stmt_t *src = other->real;
        dst->path = node_deep_copy(src->path);
        dst->alias = node_deep_copy(src->alias);
        self->real = dst;
    } break;
    case NODE_TYPE_FROM_IMPORT_STMT: {
        declare(node_from_import_stmt_t, dst);
        node_from_import_stmt_t *src = other->real;
        dst->path = node_deep_copy(src->path);
        dst->import_vars = node_deep_copy(src->import_vars);
        self->real = dst;
    } break;
    case NODE_TYPE_IMPORT_VARS: {
        declare(node_import_vars_t, dst);
        node_import_vars_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_IMPORT_VAR: {
        declare(node_import_var_t, dst);
        node_import_var_t *src = other->real;
        dst->identifier = node_deep_copy(src->identifier);
        dst->alias = node_deep_copy(src->alias);
        self->real = dst;
    } break;
    case NODE_TYPE_IF_STMT: {
        declare(node_if_stmt_t, dst);
        node_if_stmt_t *src = other->real;
        dst->test = node_deep_copy(src->test);
        copy_node_array(dst, src, contents);
        dst->elif_stmt = node_deep_copy(src->elif_stmt);
        dst->else_stmt = node_deep_copy(src->else_stmt);
        self->real = dst;
    } break;
    case NODE_TYPE_ELIF_STMT: {
        declare(node_elif_stmt_t, dst);
        node_elif_stmt_t *src = other->real;
        dst->test = node_deep_copy(src->test);
        copy_node_array(dst, src, contents);
        dst->elif_stmt = node_deep_copy(src->elif_stmt);
        dst->else_stmt = node_deep_copy(src->else_stmt);
        self->real = dst;
    } break;
    case NODE_TYPE_ELSE_STMT: {
        declare(node_else_stmt_t, dst);
        node_else_stmt_t *src = other->real;
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case NODE_TYPE_FOR_STMT: {
        declare(node_for_stmt_t, dst);
        node_for_stmt_t *src = other->real;
        dst->init_formula = node_deep_copy(src->init_formula);
        dst->comp_formula = node_deep_copy(src->comp_formula);
        dst->update_formula = node_deep_copy(src->update_formula);
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case NODE_TYPE_BREAK_STMT: {
        declare(node_break_stmt_t, dst);
        node_break_stmt_t *src = other->real;
        dst->dummy = src->dummy;
        self->real = dst;
    } break;
    case NODE_TYPE_CONTINUE_STMT: {
        declare(node_continue_stmt_t, dst);
        node_continue_stmt_t *src = other->real;
        dst->dummy = src->dummy;
        self->real = dst;
    } break;
    case NODE_TYPE_RETURN_STMT: {
        declare(node_return_stmt_t, dst);
        node_return_stmt_t *src = other->real;
        dst->formula = node_deep_copy(src->formula);
        self->real = dst;
    } break;
    case NODE_TYPE_BLOCK_STMT: {
        declare(node_block_stmt_t, dst);
        node_block_stmt_t *src = other->real;
        dst->identifier = node_deep_copy(src->identifier);
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case NODE_TYPE_INJECT_STMT: {
        declare(node_inject_stmt_t, dst);
        node_inject_stmt_t *src = other->real;
        dst->identifier = node_deep_copy(src->identifier);
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case NODE_TYPE_STRUCT: {
        declare(node_struct_t, dst);
        node_struct_t *src = other->real;
        dst->identifier = node_deep_copy(src->identifier);
        dst->elems = node_deep_copy(src->elems);
        self->real = dst;
    } break;
    case NODE_TYPE_CONTENT: {
        declare(node_content_t, dst);
        node_content_t *src = other->real;
        dst->elems = node_deep_copy(src->elems);
        dst->blocks = node_deep_copy(src->blocks);
        self->real = dst;
    } break;
    case NODE_TYPE_FORMULA: {
        declare(node_formula_t, dst);
        node_formula_t *src = other->real;
        dst->assign_list = node_deep_copy(src->assign_list);
        dst->multi_assign = node_deep_copy(src->multi_assign);
        self->real = dst;
    } break;
    case NODE_TYPE_MULTI_ASSIGN: {
        declare(node_multi_assign_t, dst);
        node_multi_assign_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_ASSIGN_LIST: {
        declare(node_assign_list_t, dst);
        node_assign_list_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_ASSIGN: {
        declare(node_assign_t, dst);
        node_assign_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_SIMPLE_ASSIGN: {
        declare(node_simple_assign_t, dst);
        node_simple_assign_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_TEST_LIST: {
        declare(node_test_list_t, dst);
        node_test_list_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_CALL_ARGS: {
        declare(node_call_args_t, dst);
        node_call_args_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_TEST: {
        declare(node_test_t, dst);
        node_test_t *src = other->real;
        dst->or_test = node_deep_copy(src->or_test);
        self->real = dst;
    } break;
    case NODE_TYPE_OR_TEST: {
        declare(node_or_test_t, dst);
        node_or_test_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_AND_TEST: {
        declare(node_and_test_t, dst);
        node_and_test_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_NOT_TEST: {
        declare(node_not_test_t, dst);
        node_not_test_t *src = other->real;
        dst->not_test = node_deep_copy(src->not_test);
        dst->comparison = node_deep_copy(src->comparison);
        self->real = dst;
    } break;
    case NODE_TYPE_COMPARISON: {
        declare(node_comparison_t, dst);
        node_comparison_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_EXPR: {
        declare(node_expr_t, dst);
        node_expr_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_TERM: {
        declare(node_term_t, dst);
        node_term_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_NEGATIVE: {
        declare(node_negative_t, dst);
        node_negative_t *src = other->real;
        dst->is_negative = src->is_negative;
        dst->chain = node_deep_copy(src->chain);
        self->real = dst;
    } break;
    case NODE_TYPE_CHAIN: {
        declare(node_chain_t, dst);
        node_chain_t *src = other->real;
        dst->factor = node_deep_copy(src->factor);
        dst->chain_nodes = chain_nodes_deep_copy(src->chain_nodes);
        self->real = dst;
    } break;
    case NODE_TYPE_ASSCALC: {
        declare(node_asscalc_t, dst);
        node_asscalc_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_FACTOR: {
        declare(node_factor_t, dst);
        node_factor_t *src = other->real;
        dst->atom = node_deep_copy(src->atom);
        dst->formula = node_deep_copy(src->formula);
        self->real = dst;
    } break;
    case NODE_TYPE_ATOM: {
        declare(node_atom_t, dst);
        node_atom_t *src = other->real;
        dst->nil = node_deep_copy(src->nil);
        dst->true_ = node_deep_copy(src->true_);
        dst->false_ = node_deep_copy(src->false_);
        dst->digit = node_deep_copy(src->digit);
        dst->string = node_deep_copy(src->string);
        dst->array = node_deep_copy(src->array);
        dst->dict = node_deep_copy(src->dict);
        dst->identifier = node_deep_copy(src->identifier);
        self->real = dst;
    } break;
    case NODE_TYPE_AUGASSIGN: {
        declare(node_augassign_t, dst);
        node_augassign_t *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case NODE_TYPE_COMP_OP: {
        declare(node_comp_op_t, dst);
        node_comp_op_t *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case NODE_TYPE_ADD_SUB_OP: {
        declare(node_add_sub_op_t, dst);
        node_add_sub_op_t *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case NODE_TYPE_MUL_DIV_OP: {
        declare(node_mul_div_op_t, dst);
        node_mul_div_op_t *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case NODE_TYPE_DOT_OP: {
        declare(node_dot_op_t, dst);
        node_dot_op_t *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case NODE_TYPE_NIL: {
        declare(node_nil_t, dst);
        node_nil_t *src = other->real;
        dst->dummy = src->dummy;
        self->real = dst;
    } break;
    case NODE_TYPE_DIGIT: {
        declare(node_digit_t, dst);
        node_digit_t *src = other->real;
        dst->lvalue = src->lvalue;
        self->real = dst;
    } break;
    case NODE_TYPE_STRING: {
        declare(node_string_t, dst);
        node_string_t *src = other->real;
        dst->string = cstr_edup(src->string);
        self->real = dst;
    } break;
    case NODE_TYPE_IDENTIFIER: {
        declare(node_identifier_t, dst);
        node_identifier_t *src = other->real;
        dst->identifier = cstr_edup(src->identifier);
        self->real = dst;
    } break;
    case NODE_TYPE_ARRAY: {
        declare(node_array_t_, dst);
        node_array_t_ *src = other->real;
        dst->array_elems = node_deep_copy(src->array_elems);
        self->real = dst;
    } break;
    case NODE_TYPE_ARRAY_ELEMS: {
        declare(node_array_elems_t, dst);
        node_array_elems_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_DICT: {
        declare(_node_dict_t, dst);
        _node_dict_t *src = other->real;
        dst->dict_elems = node_deep_copy(src->dict_elems);
        self->real = dst;
    } break;
    case NODE_TYPE_DICT_ELEMS: {
        declare(node_dict_elems_t, dst);
        node_dict_elems_t *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case NODE_TYPE_DICT_ELEM: {
        declare(node_dict_elem_t, dst);
        node_dict_elem_t *src = other->real;
        dst->key_simple_assign = node_deep_copy(src->key_simple_assign);
        dst->value_simple_assign = node_deep_copy(src->value_simple_assign);
        self->real = dst;
    } break;
    case NODE_TYPE_DEF: {
        declare(node_def_t, dst);
        node_def_t *src = other->real;
        dst->func_def = node_deep_copy(src->func_def);
        self->real = dst;
    } break;
    case NODE_TYPE_FUNC_DEF: {
        declare(node_func_def_t, dst);
        node_func_def_t *src = other->real;
        dst->identifier = node_deep_copy(src->identifier);
        dst->func_def_params = node_deep_copy(src->func_def_params);
        dst->func_extends = node_deep_copy(src->func_extends);
        copy_node_array(dst, src, contents);
        copy_node_dict(dst, src, blocks);
        self->real = dst;
    } break;
    case NODE_TYPE_FUNC_DEF_PARAMS: {
        declare(node_func_def_params_t, dst);
        node_func_def_params_t *src = other->real;
        dst->func_def_args = node_deep_copy(src->func_def_args);
        self->real = dst;
    } break;
    case NODE_TYPE_FUNC_DEF_ARGS: {
        declare(node_func_def_args_t, dst);
        node_func_def_args_t *src = other->real;
        copy_node_array(dst, src, identifiers);
        self->real = dst;
    } break;
    case NODE_TYPE_FUNC_EXTENDS: {
        declare(node_func_extends_t, dst);
        node_func_extends_t *src = other->real;
        dst->identifier = node_deep_copy(src->identifier);
        self->real = dst;
    } break;
    case NODE_TYPE_FALSE: {
        declare(node_false_t, dst);
        node_false_t *src = other->real;
        dst->boolean = src->boolean;
        self->real = dst;
    } break;
    case NODE_TYPE_TRUE: {
        declare(node_true_t, dst);
        node_true_t *src = other->real;
        dst->boolean = src->boolean;
        self->real = dst;
    } break;
    }

    return self;
}

node_type_t
node_getc_type(const node_t *self) {
    if (self == NULL) {
        return NODE_TYPE_INVALID;
    }

    return self->type;
}

void *
node_get_real(node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get real from node");
        return NULL;
    }
    return self->real;
}

const void *
node_getc_real(const node_t *self) {
    return node_get_real((node_t *) self);
}

string_t *
node_to_str(const node_t *self) {
    string_t *s = str_new();

    switch (self->type) {
    case NODE_TYPE_INVALID: str_set(s, "invalid"); break;
    case NODE_TYPE_PROGRAM: str_set(s, "program"); break;
    case NODE_TYPE_BLOCKS: str_set(s, "blocks"); break;
    case NODE_TYPE_CODE_BLOCK: str_set(s, "code block"); break;
    case NODE_TYPE_REF_BLOCK: str_set(s, "ref block"); break;
    case NODE_TYPE_TEXT_BLOCK: str_set(s, "text block"); break;
    case NODE_TYPE_ELEMS: str_set(s, "elems"); break;
    case NODE_TYPE_STMT: str_set(s, "stmt"); break;
    case NODE_TYPE_IMPORT_STMT: str_set(s, "import"); break;
    case NODE_TYPE_IMPORT_AS_STMT: str_set(s, "import as"); break;
    case NODE_TYPE_FROM_IMPORT_STMT: str_set(s, "from import"); break;
    case NODE_TYPE_IMPORT_VARS: str_set(s, "import vars"); break;
    case NODE_TYPE_IMPORT_VAR: str_set(s, "import var"); break;
    case NODE_TYPE_IF_STMT: str_set(s, "if"); break;
    case NODE_TYPE_ELIF_STMT: str_set(s, "elif"); break;
    case NODE_TYPE_ELSE_STMT: str_set(s, "else"); break;
    case NODE_TYPE_FOR_STMT: str_set(s, "for"); break;
    case NODE_TYPE_BREAK_STMT: str_set(s, "break"); break;
    case NODE_TYPE_CONTINUE_STMT: str_set(s, "continue"); break;
    case NODE_TYPE_RETURN_STMT: str_set(s, "return"); break;
    case NODE_TYPE_BLOCK_STMT: str_set(s, "block"); break;
    case NODE_TYPE_INJECT_STMT: str_set(s, "inject"); break;
    case NODE_TYPE_STRUCT: str_set(s, "struct"); break;
    case NODE_TYPE_CONTENT: str_set(s, "content"); break;
    case NODE_TYPE_FORMULA: str_set(s, "formula"); break;
    case NODE_TYPE_MULTI_ASSIGN: str_set(s, "multi assign"); break;
    case NODE_TYPE_ASSIGN_LIST: str_set(s, "assign list"); break;
    case NODE_TYPE_ASSIGN: str_set(s, "assign"); break;
    case NODE_TYPE_SIMPLE_ASSIGN: str_set(s, "simple assign"); break;
    case NODE_TYPE_TEST_LIST: str_set(s, "test list"); break;
    case NODE_TYPE_CALL_ARGS: str_set(s, "call args"); break;
    case NODE_TYPE_TEST: str_set(s, "test"); break;
    case NODE_TYPE_OR_TEST: str_set(s, "or test"); break;
    case NODE_TYPE_AND_TEST: str_set(s, "and test"); break;
    case NODE_TYPE_NOT_TEST: str_set(s, "not test"); break;
    case NODE_TYPE_COMPARISON: str_set(s, "comparison"); break;
    case NODE_TYPE_EXPR: str_set(s, "expr"); break;
    case NODE_TYPE_TERM: str_set(s, "term"); break;
    case NODE_TYPE_NEGATIVE: str_set(s, "negative"); break;
    case NODE_TYPE_CHAIN: str_set(s, "chain"); break;
    case NODE_TYPE_ASSCALC: str_set(s, "asscalc"); break;
    case NODE_TYPE_FACTOR: str_set(s, "factor"); break;
    case NODE_TYPE_ATOM: str_set(s, "atom"); break;
    case NODE_TYPE_AUGASSIGN: str_set(s, "augassign"); break;
    case NODE_TYPE_COMP_OP: str_set(s, "comp op"); break;
    case NODE_TYPE_NIL: str_set(s, "nil"); break;
    case NODE_TYPE_DIGIT: str_set(s, "digit"); break;
    case NODE_TYPE_STRING: str_set(s, "string"); break;
    case NODE_TYPE_IDENTIFIER: str_set(s, "identifier"); break;
    case NODE_TYPE_ARRAY: str_set(s, "array"); break;
    case NODE_TYPE_ARRAY_ELEMS: str_set(s, "array elems"); break;
    case NODE_TYPE_DICT: str_set(s, "dict"); break;
    case NODE_TYPE_DICT_ELEMS: str_set(s, "dict elems"); break;
    case NODE_TYPE_DICT_ELEM: str_set(s, "dict elem"); break;
    case NODE_TYPE_ADD_SUB_OP: str_set(s, "add sub op"); break;
    case NODE_TYPE_MUL_DIV_OP: str_set(s, "mul div op"); break;
    case NODE_TYPE_DOT_OP: str_set(s, "dot op"); break;
    case NODE_TYPE_DEF: str_set(s, "def"); break;
    case NODE_TYPE_FUNC_DEF: str_set(s, "func def"); break;
    case NODE_TYPE_FUNC_DEF_PARAMS: str_set(s, "func def params"); break;
    case NODE_TYPE_FUNC_DEF_ARGS: str_set(s, "func def args"); break;
    case NODE_TYPE_FUNC_EXTENDS: str_set(s, "func extends"); break;
    case NODE_TYPE_FALSE: str_set(s, "false"); break;
    case NODE_TYPE_TRUE: str_set(s, "true"); break;
    }

    return s;
}

void
node_dump(const node_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "node_t\n");
    fprintf(fout, "type: %d\n", self->type);
    fprintf(fout, "real: %p\n", self->real);
}