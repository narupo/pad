#include <pad/lang/opts.h>

struct PadOpts {
    dict_t *opts;
    cstring_array_t *args;
};

void
PadOpts_Del(PadOpts *self) {
    if (!self) {
        return;
    }
    
    dict_del(self->opts);
    cstrarr_del(self->args);
    free(self);
}

PadOpts *
PadOpts_New(void) {
    PadOpts *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = dict_new(100);
    if (!self->opts) {
        PadOpts_Del(self);
        return NULL;
    }

    self->args = cstrarr_new();
    if (!self->args) {
        PadOpts_Del(self);
        return NULL;
    }

    return self;
}

PadOpts *
PadOpts_DeepCopy(const PadOpts *other) {
    if (!other) {
        return NULL;
    }

    PadOpts *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = dict_deep_copy(other->opts);
    if (!self->opts) {
        PadOpts_Del(self);
        return NULL;
    }

    self->args = cstrarr_deep_copy(other->args);
    if (!self->args) {
        PadOpts_Del(self);
        return NULL;
    }

    return self;
}

PadOpts *
PadOpts_ShallowCopy(const PadOpts *other) {
    if (!other) {
        return NULL;
    }

    PadOpts *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = dict_shallow_copy(other->opts);
    if (!self->opts) {
        PadOpts_Del(self);
        return NULL;
    }

    self->args = cstrarr_shallow_copy(other->args);
    if (!self->args) {
        PadOpts_Del(self);
        return NULL;
    }

    return self;    
}

void
PadOpts_Clear(PadOpts *self) {
    if (!self) {
        return;
    }

    dict_clear(self->opts);
    cstrarr_clear(self->args);
}

PadOpts *
PadOpts_Parse(PadOpts *self, int argc, char *argv[]) {
    if (!self || !argv) {
        return NULL;
    }
    if (argc <= 0) {
        return self;
    }

    int m = 0;
    string_t *key = str_new();
    if (!key) {
        return NULL;
    }

    cstrarr_clear(self->args);
    cstrarr_pushb(self->args, argv[0]);

    for (int i = 1; i < argc && argv[i]; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            if (arg[0] == '-' && arg[1] == '-') {
                str_set(key, arg+2);
                m = 10;
            } else if (arg[0] == '-') {
                str_set(key, arg+1);
                m = 20;
            } else {
                if (!cstrarr_pushb(self->args, arg)) {
                    return NULL;
                }
            }
            break;
        case 10: // found long option
            if (arg[0] == '-' && arg[1] == '-') {
                dict_set(self->opts, str_getc(key), "");
                str_set(key, arg+2);
                // keep current mode
            } else if (arg[0] == '-') {
                dict_set(self->opts, str_getc(key), "");
                str_set(key, arg+1);
                m = 20;
            } else {
                // store option value
                dict_set(self->opts, str_getc(key), arg);
                str_clear(key);
                m = 0;
            }
            break;
        case 20: // found short option
            if (arg[0] == '-' && arg[1] == '-') {
                dict_set(self->opts, str_getc(key), "");
                str_set(key, arg+2);
                m = 10;
            } else if (arg[0] == '-') {
                dict_set(self->opts, str_getc(key), "");
                str_set(key, arg+1);
                // keep current mode
            } else {
                // store option value
                dict_set(self->opts, str_getc(key), arg);
                str_clear(key);
                m = 0;
            }
            break;
        }
    }

    if (str_len(key)) {
        dict_set(self->opts, str_getc(key), "");
    }

    str_del(key);
    return self;
}

const char *
PadOpts_Getc(const PadOpts *self, const char *optname) {
    if (!self || !optname) {
        return NULL;
    }

    const dict_item_t *item = dict_getc(self->opts, optname);
    if (!item) {
        return NULL;
    }
    return item->value;
}

bool
PadOpts_Has(const PadOpts *self, const char *optname) {
    if (!self || !optname) {
        return NULL;
    }

    return dict_has_key(self->opts, optname);
}

const char *
PadOpts_GetcArgs(const PadOpts *self, int32_t idx) {
    if (!self) {
        return NULL;
    }
    if (idx < 0 || idx >= cstrarr_len(self->args)) {
        return NULL;
    }

    return cstrarr_getc(self->args, idx);
}

int32_t
PadOpts_ArgsLen(const PadOpts *self) {
    if (!self) {
        return -1;
    }
    return cstrarr_len(self->args);
}
