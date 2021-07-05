#include <pad/lib/cmdline.h>

/*****************
* cmdline_object *
*****************/

void
cmdlinePadObj_Del(cmdline_PadObj *self) {
    if (!self) {
        return;
    }

    str_del(self->command);
    cl_del(self->cl);
    free(self);
}

cmdline_PadObj *
cmdlinePadObj_New(cmdline_PadObjype_t type) {
    cmdline_PadObj *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->command = str_new();
    if (!self->command) {
        cmdlinePadObj_Del(self);
        return NULL;
    }

    self->cl = cl_new();
    if (!self->cl) {
        cmdlinePadObj_Del(self);
        return NULL;
    }

    return self;
}

cmdline_PadObj *
cmdlineobj_parse(cmdline_PadObj *self, const char *line) {
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
    cmdline_PadObj **objs;
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
        cmdline_PadObj *obj = self->objs[i];
        cmdlinePadObj_Del(obj);
    }

    free(self->objs);
    free(self);
}

cmdline_t *
cmdline_new(void) {
    cmdline_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    int32_t size = sizeof(cmdline_PadObj *);
    self->capa = CMDLINE_OBJS_SIZE;
    self->objs = mem_calloc(self->capa + 1, size);
    if (!self->objs) {
        cmdline_del(self);
        return NULL;
    } 

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
            cmdlinePadObj_Del(self->objs[i]);
            self->objs[i] = NULL;
        }
    }

    int32_t objsize = sizeof(cmdline_PadObj *);
    int32_t size = objsize * capa + objsize;
    cmdline_PadObj **tmp = mem_realloc(self->objs, size);
    if (!tmp) {
        return NULL;
    }
    self->objs = tmp;
    self->capa = capa;

    return self;
}

cmdline_t *
cmdline_moveb(cmdline_t *self, cmdline_PadObj *move_obj) {
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

const cmdline_PadObj *
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
        cmdline_PadObj *obj = self->objs[i];
        cmdlinePadObj_Del(obj);
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
                cmdline_PadObj *obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_CMD);
                if (!cmdlineobj_parse(obj, str_getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    str_del(buf);
                    return NULL;
                }

                string_t *copied = str_strip(buf, " ");
                str_del(buf);
                buf = copied;

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
                obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_PIPE);

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
                cmdline_PadObj *obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_CMD);
                if (!cmdlineobj_parse(obj, str_getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    str_del(buf);
                    return NULL;
                }

                string_t *copied = str_strip(buf, " ");
                str_del(buf);
                buf = copied;

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
                obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_AND);

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
                cmdline_PadObj *obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_CMD);
                if (!cmdlineobj_parse(obj, str_getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    str_del(buf);
                    return NULL;
                }

                string_t *copied = str_strip(buf, " ");
                str_del(buf);
                buf = copied;

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
                obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_REDIRECT);

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
        cmdline_PadObj *obj = cmdlinePadObj_New(CMDLINE_OBJECT_TYPE_CMD);
        if (!cmdlineobj_parse(obj, str_getc(buf))) {
            snprintf(self->what, sizeof self->what, "failed to parse mini command line (2)");
            cmdlinePadObj_Del(obj);
            str_del(buf);
            return NULL;
        }

        string_t *copied = str_strip(buf, " ");
        str_del(buf);
        buf = copied;
        
        str_del(obj->command);
        obj->command = mem_move(buf);
        buf = NULL;

        if (!cmdline_moveb(self, obj)) {
            snprintf(self->what, sizeof self->what, "failed to move back command object (2)");
            cmdlinePadObj_Del(obj);
            str_del(buf);
            return NULL;
        }
    }

    str_del(buf);
    return self;
}
