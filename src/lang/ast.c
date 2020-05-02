#include <lang/ast.h>

void
ast_del_nodes(const ast_t *self, node_t *node) {
    if (!node) {
        return;
    }

    switch (node->type) {
    default: {
        err_die(
            "impossible. failed to delete nodes in ast. "
            "not supported node type '%d'", node->type
        );
    } break;
    case NODE_TYPE_INVALID: {
        // nothing todo
    } break;
    case NODE_TYPE_PROGRAM: {
        node_program_t *program = node->real;
        ast_del_nodes(self, program->blocks);
    } break;
    case NODE_TYPE_BLOCKS: {
        node_blocks_t *blocks = node->real;
        ast_del_nodes(self, blocks->code_block);
        ast_del_nodes(self, blocks->ref_block);
        ast_del_nodes(self, blocks->text_block);
        ast_del_nodes(self, blocks->blocks);
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        node_code_block_t *code_block = node->real;
        ast_del_nodes(self, code_block->elems);
    } break;
    case NODE_TYPE_REF_BLOCK: {
        node_ref_block_t *ref_block = node->real;
        ast_del_nodes(self, ref_block->formula);
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        node_text_block_t *text_block = node->real;
        free(text_block->text);
    } break;
    case NODE_TYPE_ELEMS: {
        node_elems_t *elems = node->real;
        ast_del_nodes(self, elems->def);
        ast_del_nodes(self, elems->stmt);
        ast_del_nodes(self, elems->formula);
        ast_del_nodes(self, elems->elems);
    } break;
    case NODE_TYPE_STMT: {
        node_stmt_t *stmt = node->real;
        ast_del_nodes(self, stmt->import_stmt);
        ast_del_nodes(self, stmt->if_stmt);
        ast_del_nodes(self, stmt->for_stmt);
        ast_del_nodes(self, stmt->break_stmt);
        ast_del_nodes(self, stmt->continue_stmt);
        ast_del_nodes(self, stmt->return_stmt);
    } break;
    case NODE_TYPE_IMPORT_STMT: {
        node_import_stmt_t *import_stmt = node->real;
        ast_del_nodes(self, import_stmt->import_as_stmt);
        ast_del_nodes(self, import_stmt->from_import_stmt);
    } break;
    case NODE_TYPE_IMPORT_AS_STMT: {
        node_import_as_stmt_t *import_as_stmt = node->real;
        ast_del_nodes(self, import_as_stmt->path);
        ast_del_nodes(self, import_as_stmt->alias);
    } break;
    case NODE_TYPE_FROM_IMPORT_STMT: {
        node_from_import_stmt_t *from_import_stmt = node->real;
        ast_del_nodes(self, from_import_stmt->path);
        ast_del_nodes(self, from_import_stmt->import_vars);
    } break;
    case NODE_TYPE_IMPORT_VARS: {
        node_import_vars_t *import_vars = node->real;
        for (int32_t i = 0; i < nodearr_len(import_vars->nodearr); ++i) {
            node_t *node = nodearr_get(import_vars->nodearr, i);
            ast_del_nodes(self, node);
        }
    } break;
    case NODE_TYPE_IMPORT_VAR: {
        node_import_var_t *import_var = node->real;
        ast_del_nodes(self, import_var->identifier);
        ast_del_nodes(self, import_var->alias);
    } break;
    case NODE_TYPE_IF_STMT: {
        node_if_stmt_t *if_stmt = node->real;
        ast_del_nodes(self, if_stmt->test);
        ast_del_nodes(self, if_stmt->elif_stmt);
        ast_del_nodes(self, if_stmt->else_stmt);
        nodearr_del(if_stmt->contents);
    } break;
    case NODE_TYPE_ELIF_STMT: {
        node_elif_stmt_t *elif_stmt = node->real;
        ast_del_nodes(self, elif_stmt->test);
        ast_del_nodes(self, elif_stmt->elif_stmt);
        ast_del_nodes(self, elif_stmt->else_stmt);
        nodearr_del(elif_stmt->contents);
    } break;
    case NODE_TYPE_ELSE_STMT: {
        node_else_stmt_t *else_stmt = node->real;
        nodearr_del(else_stmt->contents);
    } break;
    case NODE_TYPE_FOR_STMT: {
        node_for_stmt_t *for_stmt = node->real;
        ast_del_nodes(self, for_stmt->init_formula);
        ast_del_nodes(self, for_stmt->comp_formula);
        ast_del_nodes(self, for_stmt->update_formula);
        nodearr_del(for_stmt->contents);
    } break;
    case NODE_TYPE_BREAK_STMT: {
        // nothing todo
    } break;
    case NODE_TYPE_CONTINUE_STMT: {
        // nothing todo
    } break;
    case NODE_TYPE_RETURN_STMT: {
        node_return_stmt_t *return_stmt = node->real;
        ast_del_nodes(self, return_stmt->formula);
    } break;
    case NODE_TYPE_FORMULA: {
        node_formula_t *formula = node->real;
        ast_del_nodes(self, formula->assign_list);
        ast_del_nodes(self, formula->multi_assign);
    } break;
    case NODE_TYPE_MULTI_ASSIGN: {
        node_multi_assign_t *multi_assign = node->real;
        for (int32_t i = 0; i < nodearr_len(multi_assign->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(multi_assign->nodearr, i));
        }
        nodearr_del_without_nodes(multi_assign->nodearr);
    } break;
    case NODE_TYPE_ASSIGN_LIST: {
        node_assign_list_t *assign_list = node->real;
        for (int32_t i = 0; i < nodearr_len(assign_list->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(assign_list->nodearr, i));
        }
        nodearr_del_without_nodes(assign_list->nodearr);
    } break;
    case NODE_TYPE_ASSIGN: {
        node_assign_t *assign = node->real;
        for (int32_t i = 0; i < nodearr_len(assign->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(assign->nodearr, i));
        }
        nodearr_del_without_nodes(assign->nodearr);
    } break;
    case NODE_TYPE_SIMPLE_ASSIGN: {
        node_simple_assign_t *simple_assign = node->real;
        for (int32_t i = 0; i < nodearr_len(simple_assign->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(simple_assign->nodearr, i));
        }
        nodearr_del_without_nodes(simple_assign->nodearr);
    } break;
    case NODE_TYPE_TEST_LIST: {
        node_test_list_t *test_list = node->real;
        for (int32_t i = 0; i < nodearr_len(test_list->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(test_list->nodearr, i));
        }
        nodearr_del_without_nodes(test_list->nodearr);
    } break;
    case NODE_TYPE_CALL_ARGS: {
        node_test_list_t *call_args = node->real;
        for (int32_t i = 0; i < nodearr_len(call_args->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(call_args->nodearr, i));
        }
        nodearr_del_without_nodes(call_args->nodearr);
    } break;
    case NODE_TYPE_TEST: {
        node_test_t *test = node->real;
        ast_del_nodes(self, test->or_test);
    } break;
    case NODE_TYPE_OR_TEST: {
        node_or_test_t *or_test = node->real;
        for (int32_t i = 0; i < nodearr_len(or_test->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(or_test->nodearr, i));
        }
        nodearr_del_without_nodes(or_test->nodearr);
    } break;
    case NODE_TYPE_AND_TEST: {
        node_and_test_t *and_test = node->real;
        for (int32_t i = 0; i < nodearr_len(and_test->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(and_test->nodearr, i));
        }
        nodearr_del_without_nodes(and_test->nodearr);
    } break;
    case NODE_TYPE_NOT_TEST: {
        node_not_test_t *not_test = node->real;
        ast_del_nodes(self, not_test->not_test);
        ast_del_nodes(self, not_test->comparison);
    } break;
    case NODE_TYPE_COMPARISON: {
        node_comparison_t *comparison = node->real;
        for (int32_t i = 0; i < nodearr_len(comparison->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(comparison->nodearr, i));
        }
        nodearr_del_without_nodes(comparison->nodearr);
    } break;
    case NODE_TYPE_ASSCALC: {
        node_asscalc_t *asscalc = node->real;
        for (int32_t i = 0; i < nodearr_len(asscalc->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(asscalc->nodearr, i));
        }
        nodearr_del_without_nodes(asscalc->nodearr);
    } break;
    case NODE_TYPE_EXPR: {
        node_expr_t *expr = node->real;
        for (int32_t i = 0; i < nodearr_len(expr->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(expr->nodearr, i));
        }
        nodearr_del_without_nodes(expr->nodearr);
    } break;
    case NODE_TYPE_TERM: {
        node_expr_t *term = node->real;
        for (int32_t i = 0; i < nodearr_len(term->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(term->nodearr, i));
        }
        nodearr_del_without_nodes(term->nodearr);
    } break;
    case NODE_TYPE_NEGATIVE: {
        node_negative_t *negative = node->real;
        ast_del_nodes(self, negative->dot);
    } break;
    case NODE_TYPE_DOT: {
        node_dot_t *dot = node->real;
        for (int32_t i = 0; i < nodearr_len(dot->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(dot->nodearr, i));
        }
        nodearr_del_without_nodes(dot->nodearr);
    } break;
    case NODE_TYPE_CALL: {
        node_call_t *call = node->real;
        ast_del_nodes(self, call->index);
        for (int32_t i = 0; i < nodearr_len(call->call_args_list); ++i) {
            ast_del_nodes(self, nodearr_get(call->call_args_list, i));
        }
        nodearr_del_without_nodes(call->call_args_list);
    } break;
    case NODE_TYPE_INDEX: {
        node_index_t *index = node->real;
        ast_del_nodes(self, index->factor);
        for (int32_t i = 0; i < nodearr_len(index->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(index->nodearr, i));
        }
        nodearr_del_without_nodes(index->nodearr);
    } break;
    case NODE_TYPE_FACTOR: {
        node_factor_t *factor = node->real;
        ast_del_nodes(self, factor->atom);
        ast_del_nodes(self, factor->formula);
    } break;
    case NODE_TYPE_ATOM: {
        node_atom_t *atom = node->real;
        ast_del_nodes(self, atom->nil);
        ast_del_nodes(self, atom->true_);
        ast_del_nodes(self, atom->false_);
        ast_del_nodes(self, atom->digit);
        ast_del_nodes(self, atom->string);
        ast_del_nodes(self, atom->array);
        ast_del_nodes(self, atom->dict);
        ast_del_nodes(self, atom->identifier);
    } break;
    case NODE_TYPE_NIL: {
        // nothing todo
    } break;
    case NODE_TYPE_FALSE: {
        // nothing todo
    } break;
    case NODE_TYPE_TRUE: {
        // nothing todo
    } break;
    case NODE_TYPE_DIGIT: {
        // nothing todo
    } break;
    case NODE_TYPE_STRING: {
        node_string_t *string = node->real;
        free(string->string);
    } break;
    case NODE_TYPE_ARRAY: {
        node_array_t_ *array = node->real;
        ast_del_nodes(self, array->array_elems);
    } break;
    case NODE_TYPE_ARRAY_ELEMS: {
        node_array_elems_t *array_elems = node->real;
        for (int32_t i = 0; i < nodearr_len(array_elems->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(array_elems->nodearr, i));
        }
        nodearr_del_without_nodes(array_elems->nodearr);
    } break;
    case NODE_TYPE_DICT: {
        node_dict_t *dict = node->real;
        ast_del_nodes(self, dict->dict_elems);
    } break;
    case NODE_TYPE_DICT_ELEMS: {
        node_dict_elems_t *dict_elems = node->real;
        for (int32_t i = 0; i < nodearr_len(dict_elems->nodearr); ++i) {
            ast_del_nodes(self, nodearr_get(dict_elems->nodearr, i));
        }
        nodearr_del_without_nodes(dict_elems->nodearr);
    } break;
    case NODE_TYPE_DICT_ELEM: {
        node_dict_elem_t *dict_elem = node->real;
        ast_del_nodes(self, dict_elem->key_simple_assign);
        ast_del_nodes(self, dict_elem->value_simple_assign);
    } break;
    case NODE_TYPE_IDENTIFIER: {
        node_identifier_t *identifier = node->real;
        free(identifier->identifier);
    } break;
    case NODE_TYPE_COMP_OP: {
        // nothing todo
    } break;
    case NODE_TYPE_ADD_SUB_OP: {
        // nothing todo
    } break;
    case NODE_TYPE_MUL_DIV_OP: {
        // nothing todo
    } break;
    case NODE_TYPE_DOT_OP: {
        // nothing todo
    } break;
    case NODE_TYPE_AUGASSIGN: {
        // nothing todo
    } break;
    case NODE_TYPE_DEF: {
        node_def_t *def = node->real;
        ast_del_nodes(self, def->func_def);
    } break;
    case NODE_TYPE_FUNC_DEF: {
        node_func_def_t *func_def = node->real;
        ast_del_nodes(self, func_def->identifier);
        ast_del_nodes(self, func_def->func_def_params);
        for (int32_t i = 0; i < nodearr_len(func_def->contents); ++i) {
            node_t *content = nodearr_get(func_def->contents, i);
            ast_del_nodes(self, content);
        }
        nodearr_del_without_nodes(func_def->contents);
    } break;
    case NODE_TYPE_FUNC_DEF_PARAMS: {
        node_func_def_params_t *func_def_params = node->real;
        ast_del_nodes(self, func_def_params->func_def_args);
    } break;
    case NODE_TYPE_FUNC_DEF_ARGS: {
        node_func_def_args_t *func_def_args = node->real;
        for (int32_t i = 0; i < nodearr_len(func_def_args->identifiers); ++i) {
            node_t *identifier = nodearr_get(func_def_args->identifiers, i);
            ast_del_nodes(self, identifier);
        }
        nodearr_del_without_nodes(func_def_args->identifiers);
    } break;
    }

    node_del(node);
}

void
ast_del(ast_t *self) {
    if (!self) {
        return;
    }

    ast_del_nodes(self, self->root);
    opts_del(self->opts);
    errstack_del(self->error_stack);
    free(self);
}

ast_t *
ast_new(const config_t *ref_config) {
    ast_t *self = mem_ecalloc(1, sizeof(*self));

    self->ref_config = ref_config;
    self->opts = opts_new();
    self->error_stack = errstack_new();

    return self;
}

void
ast_move_opts(ast_t *self, opts_t *move_opts) {
    if (self->opts) {
        opts_del(self->opts);
    }

    self->opts = mem_move(move_opts);
}

const node_t *
ast_getc_root(const ast_t *self) {
    return self->root;
}

static void
ast_show_debug(const ast_t *self, const char *funcname) {
    if (self->debug) {
        token_t *t = *self->ptr;
        printf("debug: %s: token type[%d]\n", funcname, (t ? t->type : -1));
    }
}

void
ast_clear(ast_t *self) {
    // self->ref_config
    // do not null clear

    self->tokens = NULL;  // do not delete
    self->ptr = NULL;  // do not delete

    ast_del_nodes(self, self->root);
    self->root = NULL;  // deleted

    self->context = NULL; // do not delete

    opts_clear(self->opts);
    // do not null clear

    self->ref_gc = NULL;  // do not delete
    self->ref_dot_owner = NULL;  // do not delete
    self->import_level = 0;

    errstack_clear(self->error_stack);
    // do not null clear

    self->debug = false;  // reset
}

const char *
ast_getc_last_error_message(const ast_t *self) {
    if (!errstack_len(self->error_stack)) {
        return NULL;
    }

    const errelem_t *elem = errstack_getc(self->error_stack, errstack_len(self->error_stack)-1);
    return elem->message;
}

const char *
ast_getc_first_error_message(const ast_t *self) {
    if (!errstack_len(self->error_stack)) {
        return NULL;
    }

    const errelem_t *elem = errstack_getc(self->error_stack, 0);
    return elem->message;
}

bool
ast_has_errors(const ast_t *self) {
    return errstack_len(self->error_stack);
}

void
ast_set_debug(ast_t *self, bool debug) {
    self->debug = debug;
}

void
ast_trace_error_stack(const ast_t *self, FILE *fout) {
    errstack_trace(self->error_stack, fout);
}

void
ast_dump(const ast_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "ast[%p]\n", self);
    fprintf(fout, "ref_dot_owner\n");
    obj_dump(self->ref_dot_owner, fout);
    ctx_dump(self->context, fout);
}