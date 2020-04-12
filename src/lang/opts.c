#include <lang/opts.h>

struct opts {
    dict_t *opts;
    cstring_array_t *args;
};

void
opts_del(opts_t *self) {
    if (!self) {
        return;
    }
    dict_del(self->opts);
    cstrarr_del(self->args);
    free(self);
}

opts_t *
opts_new(void) {
    opts_t *self = mem_ecalloc(1, sizeof(*self));

    self->opts = dict_new(100);
    self->args = cstrarr_new();

    return self;
}

opts_t *
opts_parse(opts_t *self, int argc, char *argv[]) {
    if (!self || !argv) {
        return NULL;
    }
    if (argc <= 0) {
        return self;
    }
    
    int m = 0;
    string_t *key = str_new();

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
                cstrarr_pushb(self->args, arg);
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
opts_getc(const opts_t *self, const char *optname) {
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
opts_has(const opts_t *self, const char *optname) {
    if (!self || !optname) {
        return NULL;
    }

    return dict_has_key(self->opts, optname);
}

const char *
opts_getc_args(const opts_t *self, int32_t idx) {
    if (!self) {
        return NULL;
    }
    if (idx < 0 || idx >= cstrarr_len(self->args)) {
        return NULL;
    }

    return cstrarr_getc(self->args, idx);
}

int32_t 
opts_args_len(const opts_t *self) {
    if (!self) {
        return -1;
    }
    return cstrarr_len(self->args);
}