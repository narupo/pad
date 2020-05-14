#include <lib/cmdline.h>

/*****************
* cmdline_object *
*****************/

void
cmdlineobj_del(cmdline_object_t *self) {
    if (!self) {
        return;
    }

    str_del(self->command);
    cl_del(self->cl);
    free(self);
}

cmdline_object_t *
cmdlineobj_new(cmdline_object_type_t type) {
    cmdline_object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = type;
    self->command = str_new();
    self->cl = cl_new();

    return self;
}

cmdline_object_t *
cmdlineobj_parse(cmdline_object_t *self, const char *line) {
    if (!cl_parse_str(self->cl, line)) {
        return NULL;
    }

    return self;
}

/**********
* cmdline *
**********/

enum {
    CMDLINE_OBJS_SIZE = 4,
};

struct cmdline {
    cmdline_object_t **objs;
    int32_t capa;
    int32_t len;
    char what[1024];
};

void
cmdline_del(cmdline_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        cmdline_object_t *obj = self->objs[i];
        cmdlineobj_del(obj);
    }

    free(self->objs);
    free(self);
}

cmdline_t *
cmdline_new(void) {
    cmdline_t *self = mem_ecalloc(1, sizeof(*self));

    int32_t size = sizeof(cmdline_object_t *);
    self->capa = CMDLINE_OBJS_SIZE;
    self->objs = mem_ecalloc(self->capa + 1, size);

    return self;
}

cmdline_t *
cmdline_resize(cmdline_t *self, int32_t capa) {
    if (!self || capa <= 0) {
        return NULL;
    }

    if (capa < self->len) {
        int32_t dif = self->len - capa;
        for (int32_t i = dif; i < self->len; ++i) {
            cmdlineobj_del(self->objs[i]);
            self->objs[i] = NULL;
        }
    }

    int32_t objsize = sizeof(cmdline_object_t *);
    int32_t size = objsize * capa + objsize;
    self->objs = mem_erealloc(self->objs, size);
    self->capa = capa;

    return self;
}

cmdline_t *
cmdline_moveb(cmdline_t *self, cmdline_object_t *move_obj) {
    if (!self || !move_obj) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!cmdline_resize(self, self->capa*2)) {
            return NULL;
        }
    }

    self->objs[self->len++] = mem_move(move_obj);

    return self;
}

bool
cmdline_has_error(const cmdline_t *self) {
    if (!self) {
        return true;
    }

    return self->what[0] != '\0';
}

int32_t
cmdline_len(const cmdline_t *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

const cmdline_object_t *
cmdline_getc(const cmdline_t *self, int32_t index) {
    if (!self || index < 0 || index >= self->len) {
        return NULL;
    }

    return self->objs[index];
}

void
cmdline_clear(cmdline_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        cmdline_object_t *obj = self->objs[i];
        cmdlineobj_del(obj);
        self->objs[i] = NULL;
    }

    self->len = 0;
    self->what[0] = '\0';
}

cmdline_t *
cmdline_parse(cmdline_t *self, const char *line) {
    if (!self || !line) {
        return NULL;
    }

    string_t *buf = str_new();
    int32_t m = 0;
    cmdline_clear(self);

    for (const char *p = line; *p; ++p) {
        switch (m) {
        case 0: // first
            if (*p == '\\') {
                ++p;
                if (*p != '\0') {
                    str_pushb(buf, *p);
                } else {
                    --p;
                }
            } else if (*p == '"') {
                str_pushb(buf, *p);
                m = 10;
            } else if (*p == '|') {
                if (!str_len(buf)) {
                    snprintf(self->what, sizeof self->what, "invalid command line");
                    str_del(buf);
                    return NULL;
                }

                // move back cmd object
                cmdline_object_t *obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_CMD);
                if (!cmdlineobj_parse(obj, str_getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    str_del(buf);
                    return NULL;
                }

                str_strip(buf, " ");
                str_del(obj->command);
                obj->command = mem_move(buf);
                buf = str_new();

                if (!cmdline_moveb(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back command object");
                    str_del(buf);
                    return NULL;
                }
                obj = NULL;

                // move back pipe object
                obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_PIPE);

                if (!cmdline_moveb(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back PIPE object");
                    str_del(buf);
                    return NULL;
                }
            } else if (*p == '&' && *(p+1) == '&') {
                ++p;

                if (!str_len(buf)) {
                    snprintf(self->what, sizeof self->what, "invalid command line (2)");
                    str_del(buf);
                    return NULL;
                }

                // move back cmd object
                cmdline_object_t *obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_CMD);
                if (!cmdlineobj_parse(obj, str_getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    str_del(buf);
                    return NULL;
                }

                str_strip(buf, " ");
                str_del(obj->command);
                obj->command = buf;
                buf = str_new();

                if (!cmdline_moveb(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back command object");
                    str_del(buf);
                    return NULL;
                }
                obj = NULL;

                // move back and object
                obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_AND);

                if (!cmdline_moveb(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back AND object");
                    str_del(buf);
                    return NULL;
                }
            } else if (*p == '>') {
                if (!str_len(buf)) {
                    snprintf(self->what, sizeof self->what, "invalid command line (3)");
                    str_del(buf);
                    return NULL;
                }

                // move back cmd object
                cmdline_object_t *obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_CMD);
                if (!cmdlineobj_parse(obj, str_getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    str_del(buf);
                    return NULL;
                }

                str_strip(buf, " ");
                str_del(obj->command);
                obj->command = buf;
                buf = str_new();

                if (!cmdline_moveb(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back command object");
                    str_del(buf);
                    return NULL;
                }
                obj = NULL;

                // move back and object
                obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_REDIRECT);

                if (!cmdline_moveb(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back REDIRECT object");
                    str_del(buf);
                    return NULL;
                }
            } else {
                str_pushb(buf, *p);
            }
            break;
        case 10: // found "
            if (*p == '\\') {
                ++p;
                if (*p != '\0') {
                    str_pushb(buf, *p);
                } else {
                    --p;
                }
            } else if (*p == '"') {
                str_pushb(buf, *p);
                m = 0;
            } else {
                str_pushb(buf, *p);
            }
            break;
        }
    }

    if (str_len(buf)) {
        // move back cmd object
        cmdline_object_t *obj = cmdlineobj_new(CMDLINE_OBJECT_TYPE_CMD);
        if (!cmdlineobj_parse(obj, str_getc(buf))) {
            snprintf(self->what, sizeof self->what, "failed to parse mini command line (2)");
            cmdlineobj_del(obj);
            str_del(buf);
            return NULL;
        }

        str_strip(buf, " ");
        str_del(obj->command);
        obj->command = mem_move(buf);
        buf = NULL;

        if (!cmdline_moveb(self, obj)) {
            snprintf(self->what, sizeof self->what, "failed to move back command object (2)");
            cmdlineobj_del(obj);
            str_del(buf);
            return NULL;
        }
    }

    str_del(buf);
    return self;
}
