#include <lang/utils.h>

ast_t *
get_ast_by_owners(ast_t *default_ast, object_array_t *ref_dot_owners) {
    if (!default_ast) {
        return NULL;
    }
    if (!ref_dot_owners || !objarr_len(ref_dot_owners)) {
        return default_ast;
    }

    int32_t ownslen = objarr_len(ref_dot_owners);
    object_t *owner = objarr_get(ref_dot_owners, ownslen-1);
    if (!owner) {
        return default_ast;
    }

again:
    switch (owner->type) {
    default:
        // owner is has not ast so return default ast
        return default_ast;
        break;
    case OBJ_TYPE_RESERV:
        // reservation object can't become to owner
        // this object temporary on context
        // so, this refer on this to invalid context
        ast_pushb_error(default_ast, "owner is invalid object (%d)", owner->type);
        return NULL;
        break;
    case OBJ_TYPE_MODULE:
        // module object has ast
        return owner->module.ast;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        // do not use pull_in_ref_by_owner
        // find owner object from current scope of ast
        owner = pull_in_ref_by(owner);
        if (!owner) {
            return default_ast;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

/**
 * this function do not push error at ast's error stack
 */
object_t *
pull_in_ref_by(const object_t *idn_obj) {
    assert(idn_obj->type == OBJ_TYPE_IDENTIFIER);

    ast_t *ref_ast = obj_get_idn_ref_ast(idn_obj);
    const char *idn = obj_getc_idn_name(idn_obj);
    object_t *ref = ctx_find_var_ref(ref_ast->ref_context, idn);
    if (!ref) {
        // do not push error stack
        return NULL;
    }
    if (ref->type == OBJ_TYPE_IDENTIFIER) {
        return pull_in_ref_by(ref);
    }

    return ref;
}

object_t *
copy_value_of_index_obj(
    ast_t *ast,
    const object_t *index_obj
) {
    assert(index_obj && index_obj->type == OBJ_TYPE_INDEX);

    assert(index_obj->index.operand);
    object_t *operand = obj_new_other(index_obj->index.operand);
    object_t *tmp_operand = NULL;
    assert(index_obj->index.indices);
    const object_array_t *indices = index_obj->index.indices;
    assert(operand);
    assert(indices);

    for (int32_t i = 0; i < objarr_len(indices); ++i) {
        const object_t *el = objarr_getc(indices, i);
        assert(el);

        const object_t *idx = el;
        if (el->type == OBJ_TYPE_IDENTIFIER) {
            idx = pull_in_ref_by(el);
            if (!idx) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined in index object",
                    obj_getc_idn_name(el)
                );
                obj_del(operand);
                return NULL;
            }
        }

        const char *skey = NULL;
        long ikey = -1;
        switch (idx->type) {
        default: err_die("invalid index type in get value of index obj"); break;
        case OBJ_TYPE_STRING: skey = str_getc(idx->string); break;
        case OBJ_TYPE_INT: ikey = idx->lvalue; break;
        }

        if (operand->type == OBJ_TYPE_IDENTIFIER) {
            object_t *ref = pull_in_ref_by(operand);
            if (ast_has_errors(ast)) {
                obj_del(operand);
                return NULL;
            } else if (!ref) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined in get value of index object",
                    obj_getc_idn_name(operand)
                );
                obj_del(operand);
                return NULL;
            }
            obj_del(operand);
            operand = obj_new_other(ref);
        }

        switch (operand->type) {
        default:
            ast_pushb_error(ast, "invalid operand type (%d) in get value of index object", operand->type);
            obj_del(operand);
            break;
        case OBJ_TYPE_ARRAY: {
            if (idx->type != OBJ_TYPE_INT) {
                ast_pushb_error(ast, "invalid array index value. value is not integer");
                obj_del(operand);
                return NULL;
            }

            if (ikey < 0 || ikey >= objarr_len(operand->objarr)) {
                ast_pushb_error(ast, "index out of range of array");
                obj_del(operand);
                return NULL;
            }

            tmp_operand = obj_new_other(objarr_getc(operand->objarr, ikey));
            obj_del(operand);
            operand = tmp_operand;
            tmp_operand = NULL;
        } break;
        case OBJ_TYPE_STRING: {
            if (idx->type != OBJ_TYPE_INT) {
                ast_pushb_error(ast, "invalid string index value. value is not integer");
                obj_del(operand);
                return NULL;
            }

            if (ikey < 0 || ikey >= str_len(operand->string)) {
                ast_pushb_error(ast, "index out of range of string");
                obj_del(operand);
                return NULL;
            }

            const char ch = str_getc(operand->string)[ikey];
            string_t *str = str_new();
            str_pushb(str, ch);

            obj_del(operand);
            operand = obj_new_str(ast->ref_gc, str);
        } break;
        case OBJ_TYPE_DICT: {
            if (idx->type != OBJ_TYPE_STRING) {
                ast_pushb_error(ast, "invalid dict index value. value is not a string");
                obj_del(operand);
                return NULL;
            }
            assert(skey);

            const object_dict_item_t *item = objdict_getc(operand->objdict, skey);
            if (!item) {
                obj_del(operand);
                return NULL;
            }

            tmp_operand = obj_new_other(item->value);
            obj_del(operand);
            operand = tmp_operand;
            tmp_operand = NULL;
        } break;
        }
    }

    return operand;
}

string_t *
obj_to_string(ast_t *ast, const object_t *obj) {
    assert(obj);

    switch (obj->type) {
    default: return obj_to_str(obj); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = pull_in_ref_by(obj);
        if (!var) {
            ast_pushb_error(
                ast,
                "\"%s\" is not defined in object to string",
                obj_getc_idn_name(obj)
            );
            return NULL;
        }
        return obj_to_str(var);
    } break;
    }

    assert(0 && "impossible. failed to ast obj to str");
    return NULL;
}

object_t *
copy_object_value(ast_t *ast, const object_t *obj) {
    assert(obj);

    object_t *copied = extract_copy_of_obj(ast, obj);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract object with copy");
        return NULL;
    }

    return copied;
}

void
move_obj_at_cur_varmap(
    ast_t *ast,
    object_array_t *ref_dot_owners,
    const char *identifier,
    object_t *move_obj
) {
    assert(move_obj->type != OBJ_TYPE_IDENTIFIER);

    ast = get_ast_by_owners(ast, ref_dot_owners);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "can't move object");
        return;
    }

    object_dict_t *varmap = ctx_get_varmap(ast->ref_context);
    objdict_move(varmap, identifier, mem_move(move_obj));
}

void
set_ref_at_cur_varmap(
    ast_t *ast,
    object_array_t *ref_dot_owners,
    const char *identifier,
    object_t *ref
) {
    assert(ref->type != OBJ_TYPE_IDENTIFIER);

    ast = get_ast_by_owners(ast, ref_dot_owners);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "can't set reference");
        return;
    }

    object_dict_t *varmap = ctx_get_varmap(ast->ref_context);
    object_t *popped = objdict_pop(varmap, identifier);
    if (popped != ref) {
        obj_inc_ref(ref);
    }

    obj_del(popped);
    objdict_set(varmap, identifier, ref);
}

object_t *
refer_index_obj_with_ref(ast_t *ast, const object_t *index_obj) {
    assert(index_obj && index_obj->type == OBJ_TYPE_INDEX);

    assert(index_obj->index.operand);
    object_t *operand = index_obj->index.operand;
    assert(index_obj->index.indices);
    const object_array_t *indices = index_obj->index.indices;
    assert(operand);
    assert(indices);

    for (int32_t i = 0; i < objarr_len(indices); ++i) {
        const object_t *el = objarr_getc(indices, i);
        assert(el);

        const object_t *idx = el;
        if (el->type == OBJ_TYPE_IDENTIFIER) {
            idx = pull_in_ref_by(el);
            if (!idx) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined in index object",
                    obj_getc_idn_name(el)
                );
                return NULL;
            }
        }

        const char *skey = NULL;
        long ikey = -1;
        switch (idx->type) {
        default: err_die("invalid index type in get value of index obj"); break;
        case OBJ_TYPE_STRING: skey = str_getc(idx->string); break;
        case OBJ_TYPE_INT: ikey = idx->lvalue; break;
        }

        if (operand->type == OBJ_TYPE_IDENTIFIER) {
            object_t *ref = pull_in_ref_by(operand);
            if (ast_has_errors(ast)) {
                return NULL;
            } else if (!ref) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined in get value of index object",
                    obj_getc_idn_name(operand)
                );
                return NULL;
            }
            operand = ref;
        }

        switch (operand->type) {
        default:
            ast_pushb_error(ast, "invalid operand type (%d) in get value of index object", operand->type);
            break;
        case OBJ_TYPE_ARRAY: {
            if (idx->type != OBJ_TYPE_INT) {
                ast_pushb_error(ast, "invalid array index value. value is not integer");
                return NULL;
            }

            if (ikey < 0 || ikey >= objarr_len(operand->objarr)) {
                ast_pushb_error(ast, "index out of range of array");
                return NULL;
            }

            operand = objarr_get(operand->objarr, ikey);
        } break;
        case OBJ_TYPE_STRING: {
            if (idx->type != OBJ_TYPE_INT) {
                ast_pushb_error(ast, "invalid string index value. value is not integer");
                return NULL;
            }

            if (ikey < 0 || ikey >= str_len(operand->string)) {
                ast_pushb_error(ast, "index out of range of string");
                return NULL;
            }

            const char ch = str_getc(operand->string)[ikey];
            string_t *str = str_new();
            str_pushb(str, ch);

            operand = obj_new_str(ast->ref_gc, str);
        } break;
        case OBJ_TYPE_DICT: {
            if (idx->type != OBJ_TYPE_STRING) {
                ast_pushb_error(ast, "invalid dict index value. value is not a string");
                return NULL;
            }
            assert(skey);

            const object_dict_item_t *item = objdict_getc(operand->objdict, skey);
            if (!item) {
                return NULL;
            }

            operand = item->value;
        } break;
        }
    }

    return operand;
}

object_t *
extract_copy_of_obj(ast_t *ast, const object_t *obj) {
    assert(ast);
    assert(obj);

    switch (obj->type) {
    default:
        return obj_new_other(obj);
        break;
    case OBJ_TYPE_RESERV:
        ast_pushb_error(ast, "can't copy from reservation object");
        return NULL;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by(obj);
        if (!ref) {
            ast_pushb_error(
                ast,
                "\"%s\" is not defined in extract obj",
                obj_getc_idn_name(obj)
            );
            return NULL;
        }
        return obj_new_other(ref);
    } break;
    case OBJ_TYPE_DICT: {
        // copy dict elements recursive
        object_dict_t *objdict = objdict_new(ast->ref_gc);

        for (int32_t i = 0; i < objdict_len(obj->objdict); ++i) {
            const object_dict_item_t *item = objdict_getc_index(obj->objdict, i);
            assert(item);
            object_t *el = item->value;
            object_t *newel = extract_copy_of_obj(ast, el);
            objdict_move(objdict, item->key, mem_move(newel));
        }

        return obj_new_dict(ast->ref_gc, objdict);
    } break;
    case OBJ_TYPE_ARRAY: {
        // copy array elements recursive
        object_array_t *objarr = objarr_new();

        for (int32_t i = 0; i < objarr_len(obj->objarr); ++i) {
            object_t *el = objarr_get(obj->objarr, i);
            object_t *newel = extract_copy_of_obj(ast, el);
            objarr_moveb(objarr, mem_move(newel));
        }

        return obj_new_array(ast->ref_gc, objarr);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *ref = refer_index_obj_with_ref(ast, obj);
        if (!ref) {
            ast_pushb_error(ast, "failed to refer index");
            return NULL;
        }
        return obj_new_other(ref);
    } break;
    }

    assert(0 && "impossible. failed to extract object");
    return NULL;
}

object_t *
extract_ref_of_obj(ast_t *ast, object_t *obj) {
    assert(ast);
    assert(obj);

    switch (obj->type) {
    default:
        return obj;
        break;
    case OBJ_TYPE_RESERV: {
        ast_pushb_error(ast, "can't extract from reservation object");
        return NULL;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by(obj);
        if (!ref) {
            ast_pushb_error(ast, "\"%s\" is not defined", obj_getc_idn_name(obj));
            return NULL;
        }
        return ref;
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *ref = refer_index_obj_with_ref(ast, obj);
        if (!ref) {
            ast_pushb_error(ast, "failed to refer index");
            return NULL;
        }
        return ref;
    } break;
    }

    assert(0 && "impossible. failed to extract reference");
    return NULL;
}

void
dump_array_obj(const object_t *arrobj) {
    assert(arrobj->type == OBJ_TYPE_ARRAY);

    object_array_t *objarr = arrobj->objarr;

    for (int32_t i = 0; i < objarr_len(objarr); ++i) {
        const object_t *obj = objarr_getc(objarr, i);
        string_t *s = obj_to_str(obj);
        printf("arr[%d] = [%s]\n", i, str_getc(s));
        str_del(s);
    }
}

bool
parse_bool(ast_t *ast, const object_t *obj) {
    if (!ast) {
        return false;
    }
    if (!obj) {
        ast_pushb_error(ast, "object is null");
        return false;
    }

    switch (obj->type) {
    default:
        return true;
        break;
    case OBJ_TYPE_RESERV:
        ast_pushb_error(ast, "can't conver to bool of reservation object");
        return false;
        break;
    case OBJ_TYPE_NIL: return false; break;
    case OBJ_TYPE_INT: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        object_t *obj = ctx_find_var_ref(ast->ref_context, idn);
        if (!obj) {
            ast_pushb_error(ast, "\"%s\" is not defined in if statement", idn);
            return false;
        }

        return parse_bool(ast, obj);
    } break;
    case OBJ_TYPE_STRING: return str_len(obj->string); break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_INDEX: {
        object_t *val = copy_value_of_index_obj(ast, obj);
        if (!val) {
            ast_pushb_error(ast, "value is null in parse bool");
            return false;
        }
        bool result = parse_bool(ast, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}
