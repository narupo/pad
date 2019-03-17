#include "modules/lang/tokens.h"

void
token_del(token_t *self) {
    if (self) {
        free(self->text);
        free(self);
    }
}

token_t *
token_new(int type) {
    token_t *self = mem_ecalloc(1, sizeof(*self));
    
    self->type = type;

    return self;
}

void
token_move_text(token_t *self, char *move_text) {
    free(self->text);
    self->text = move_text;
}

int
token_get_type(const token_t *self) {
    return self->type;
}

const char *
token_getc_text(const token_t *self) {
    return self->text;
}
