#include <lang/opts.h>

struct opts {
    dict_t *opts;    
};

void
opts_del(opts_t *self) {
    if (!self) {
        return;
    }
    dict_del(self->opts);
    free(self);
}

opts_t *
opts_new(void) {
    opts_t *self = mem_ecalloc(1, sizeof(*self));

    self->opts = dict_new(100);

    return self;
}

opts_t *
opts_parse(opts_t *self, int argc, char *argv[]) {
    if (!self || !argv) {
        return NULL;
    }
    
    int m = 0;
    string_t *key = str_new();

    for (int i = 0; i < argc && argv[i]; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            if (arg[0] == '-' && arg[1] == '-') {
                str_set(key, arg+2);
                m = 10;
            } else if (arg[0] == '-') {
                str_set(key, arg+1);
                m = 20;
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