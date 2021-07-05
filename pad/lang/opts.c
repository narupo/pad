#include <pad/lang/opts.h>

struct PadOpts {
    PadDict *opts;
    PadCStrAry *args;
};

void
PadOpts_Del(PadOpts *self) {
    if (!self) {
        return;
    }
    
    PadDict_Del(self->opts);
    PadCStrAry_Del(self->args);
    free(self);
}

PadOpts *
PadOpts_New(void) {
    PadOpts *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = PadDict_New(100);
    if (!self->opts) {
        PadOpts_Del(self);
        return NULL;
    }

    self->args = PadCStrAry_New();
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

    PadOpts *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = PadDict_DeepCopy(other->opts);
    if (!self->opts) {
        PadOpts_Del(self);
        return NULL;
    }

    self->args = PadCStrAry_DeepCopy(other->args);
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

    PadOpts *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->opts = PadDict_ShallowCopy(other->opts);
    if (!self->opts) {
        PadOpts_Del(self);
        return NULL;
    }

    self->args = PadCStrAry_ShallowCopy(other->args);
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

    PadDict_Clear(self->opts);
    PadCStrAry_Clear(self->args);
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

    PadCStrAry_Clear(self->args);
    PadCStrAry_PushBack(self->args, argv[0]);

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
                if (!PadCStrAry_PushBack(self->args, arg)) {
                    return NULL;
                }
            }
            break;
        case 10: // found long option
            if (arg[0] == '-' && arg[1] == '-') {
                PadDict_Set(self->opts, str_getc(key), "");
                str_set(key, arg+2);
                // keep current mode
            } else if (arg[0] == '-') {
                PadDict_Set(self->opts, str_getc(key), "");
                str_set(key, arg+1);
                m = 20;
            } else {
                // store option value
                PadDict_Set(self->opts, str_getc(key), arg);
                str_clear(key);
                m = 0;
            }
            break;
        case 20: // found short option
            if (arg[0] == '-' && arg[1] == '-') {
                PadDict_Set(self->opts, str_getc(key), "");
                str_set(key, arg+2);
                m = 10;
            } else if (arg[0] == '-') {
                PadDict_Set(self->opts, str_getc(key), "");
                str_set(key, arg+1);
                // keep current mode
            } else {
                // store option value
                PadDict_Set(self->opts, str_getc(key), arg);
                str_clear(key);
                m = 0;
            }
            break;
        }
    }

    if (str_len(key)) {
        PadDict_Set(self->opts, str_getc(key), "");
    }

    str_del(key);
    return self;
}

const char *
PadOpts_Getc(const PadOpts *self, const char *optname) {
    if (!self || !optname) {
        return NULL;
    }

    const PadDictItem *item = PadDict_Getc(self->opts, optname);
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
    if (idx < 0 || idx >= PadCStrAry_Len(self->args)) {
        return NULL;
    }

    return PadCStrAry_Getc(self->args, idx);
}

int32_t
PadOpts_ArgsLen(const PadOpts *self) {
    if (!self) {
        return -1;
    }
    return PadCStrAry_Len(self->args);
}
