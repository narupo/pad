#include "lang/nodes.h"

/**************
* caller_node *
**************/

void
caller_node_del(caller_node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self);
}

caller_node_t *
caller_node_new(void) {
    caller_node_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}

caller_node_t *
caller_node_pushb_identifier(caller_node_t *self, const char *identifier) {
    if (self->il_pos >= 32) {
        err_error("identifier stack overflow in caller node");
        return NULL;
    }
    char *p = self->identifier_list[self->il_pos++];
    snprintf(p, 1024, "%s", identifier);
    return self;
}

caller_node_t *
caller_node_pushb_arg(caller_node_t *self, const char *str) {
    if (self->args_pos >= 32) {
        err_error("args stack overflow in caller node");
        return NULL;
    }
    char *p = self->args[self->args_pos++];
    snprintf(p, 1024, "%s", str);
    return self;
}

const char *
caller_node_identifiers_getc(const caller_node_t *self, size_t idx) {
    if (idx >= self->il_pos) {
        return NULL;
    }
    return self->identifier_list[idx];
}

const char *
caller_node_args_getc(const caller_node_t *self, size_t idx) {
    if (idx >= self->args_pos) {
        return NULL;
    }
    return self->args[idx];    
}

size_t 
caller_node_identifiers_len(const caller_node_t *self) {
    return self->il_pos;
}

size_t
caller_node_args_len(const caller_node_t *self) {
    return self->args_pos;
}

/***************
* import_node *
***************/

void
import_node_del(import_node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self);
}

import_node_t *
import_node_new(const char *package) {
    import_node_t *self = mem_ecalloc(1, sizeof(*self));

    snprintf(self->package, sizeof(self->package), "%s", package);

    return self;
}

const char *
import_node_getc_package(const import_node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get package of import node");
        return NULL;
    }
    return self->package;
}

/**********
* formula *
**********/

void
formula_node_del(formula_node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self);
}

formula_node_t *
formula_node_new(node_t *lhs, node_t *rhs) {
    formula_node_t *self = mem_ecalloc(1, sizeof(*self));

    self->lhs = lhs;
    self->rhs = rhs;

    return self;
}

node_t *
formula_node_get_lhs(formula_node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get left hand side of formual node");
        return NULL;
    }
    return self->lhs;
}

node_t *
formula_node_get_rhs(formula_node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get right hand side of formual node");
        return NULL;
    }
    return self->rhs;
}

const node_t *
formula_node_getc_lhs(const formula_node_t *self) {
    return formula_node_get_lhs((formula_node_t *) self);
}

const node_t *
formula_node_getc_rhs(const formula_node_t *self) {
    return formula_node_get_rhs((formula_node_t *) self);
}

/******************
* text_block_node *
******************/

void
text_block_node_del(text_block_node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self->text);
    free(self);
}

text_block_node_t *
text_block_node_new(char *text) {
    text_block_node_t *self = mem_ecalloc(1, sizeof(*self));

    self->text = text;

    return self;
}

const char *
text_block_node_getc_text(const text_block_node_t *self) {
    return self->text;
}

/******************
* code_block_node *
******************/

void
code_block_node_del(code_block_node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self);
}

code_block_node_t *
code_block_node_new(node_t *formula) {
    code_block_node_t *self = mem_ecalloc(1, sizeof(*self));
    self->formula = formula;
    return self;
}

node_t *
code_block_node_get_formula(code_block_node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get formula of code block node");
        return NULL;
    }
    return self->formula;
}

const node_t *
code_block_node_getc_formula(const code_block_node_t *self) {
    return code_block_node_get_formula((code_block_node_t *) self);
}

/***********
* bin_node *
***********/

void
bin_node_del(bin_node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self);
}

bin_node_t *
bin_node_new(node_t *lhs, node_t *rhs) {
    bin_node_t *self = mem_ecalloc(1, sizeof(*self));
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

node_t *
bin_node_get_lhs(bin_node_t *self) {
    if (!self) {
        err_warn("reference to null pointer in get lhs of bin node");
        return NULL;
    }
    return self->lhs;
}

node_t *
bin_node_get_rhs(bin_node_t *self) {
    if (!self) {
        err_warn("reference to null pointer in get rhs of bin node");
        return NULL;
    }
    return self->rhs;
}

const node_t *
bin_node_getc_lhs(const bin_node_t *self) {
    if (!self) {
        err_warn("reference to null pointer in get lhs of bin node read-only");
        return NULL;
    }
    return self->lhs;
}

const node_t *
bin_node_getc_rhs(const bin_node_t *self) {
    if (!self) {
        err_warn("reference to null pointer in get rhs of bin node read-only");
        return NULL;
    }
    return self->rhs;
}

/*******
* node *
*******/

void
node_del(node_t *self) {
    if (self == NULL) {
        return;
    }
    free(self);
}

node_t *
node_new(node_type_t type, void *real) {
    node_t *self = mem_ecalloc(1, sizeof(*self));
    assert(type != 2);
    self->type = type;
    self->real = real;
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
