#include <pad/lib/cmdline.h>

/*****************
* PadCmdlineObj *
*****************/

void
PadCmdlineObj_Del(PadCmdlineObj *self) {
    if (!self) {
        return;
    }

    PadStr_Del(self->command);
    PadCL_Del(self->cl);
    free(self);
}

PadCmdlineObj *
PadCmdlineObj_New(PadCmdlineObjType type) {
    PadCmdlineObj *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->command = PadStr_New();
    if (!self->command) {
        PadCmdlineObj_Del(self);
        return NULL;
    }

    self->cl = PadCL_New();
    if (!self->cl) {
        PadCmdlineObj_Del(self);
        return NULL;
    }

    return self;
}

PadCmdlineObj *
PadCmdlineObj_Parse(PadCmdlineObj *self, const char *line) {
    if (!PadCL_ParseStr(self->cl, line)) {
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

struct PadCmdline {
    PadCmdlineObj **objs;
    int32_t capa;
    int32_t len;
    char what[1024];
};

void
PadCmdline_Del(PadCmdline *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadCmdlineObj *obj = self->objs[i];
        PadCmdlineObj_Del(obj);
    }

    free(self->objs);
    free(self);
}

PadCmdline *
PadCmdline_New(void) {
    PadCmdline *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    int32_t size = sizeof(PadCmdlineObj *);
    self->capa = CMDLINE_OBJS_SIZE;
    self->objs = PadMem_Calloc(self->capa + 1, size);
    if (!self->objs) {
        PadCmdline_Del(self);
        return NULL;
    } 

    return self;
}

PadCmdline *
PadCmdline_Resize(PadCmdline *self, int32_t capa) {
    if (!self || capa <= 0) {
        return NULL;
    }

    if (capa < self->len) {
        int32_t dif = self->len - capa;
        for (int32_t i = dif; i < self->len; ++i) {
            PadCmdlineObj_Del(self->objs[i]);
            self->objs[i] = NULL;
        }
    }

    int32_t objsize = sizeof(PadCmdlineObj *);
    int32_t size = objsize * capa + objsize;
    PadCmdlineObj **tmp = PadMem_Realloc(self->objs, size);
    if (!tmp) {
        return NULL;
    }
    self->objs = tmp;
    self->capa = capa;

    return self;
}

PadCmdline *
PadCmdline_MoveBack(PadCmdline *self, PadCmdlineObj *move_obj) {
    if (!self || !move_obj) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadCmdline_Resize(self, self->capa*2)) {
            return NULL;
        }
    }

    self->objs[self->len++] = PadMem_Move(move_obj);

    return self;
}

bool
PadCmdline_HasErr(const PadCmdline *self) {
    if (!self) {
        return true;
    }

    return self->what[0] != '\0';
}

int32_t
PadCmdline_Len(const PadCmdline *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

const PadCmdlineObj *
PadCmdline_Getc(const PadCmdline *self, int32_t index) {
    if (!self || index < 0 || index >= self->len) {
        return NULL;
    }

    return self->objs[index];
}

void
PadCmdline_Clear(PadCmdline *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadCmdlineObj *obj = self->objs[i];
        PadCmdlineObj_Del(obj);
        self->objs[i] = NULL;
    }

    self->len = 0;
    self->what[0] = '\0';
}

PadCmdline *
PadCmdline_Parse(PadCmdline *self, const char *line) {
    if (!self || !line) {
        return NULL;
    }

    PadStr *buf = PadStr_New();
    int32_t m = 0;
    PadCmdline_Clear(self);

    for (const char *p = line; *p; ++p) {
        switch (m) {
        case 0: // first
            if (*p == '\\') {
                ++p;
                if (*p != '\0') {
                    PadStr_PushBack(buf, *p);
                } else {
                    --p;
                }
            } else if (*p == '"') {
                PadStr_PushBack(buf, *p);
                m = 10;
            } else if (*p == '|') {
                if (!PadStr_Len(buf)) {
                    snprintf(self->what, sizeof self->what, "invalid command line");
                    PadStr_Del(buf);
                    return NULL;
                }

                // move back cmd object
                PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);
                if (!PadCmdlineObj_Parse(obj, PadStr_Getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    PadStr_Del(buf);
                    return NULL;
                }

                PadStr *copied = PadStr_Strip(buf, " ");
                PadStr_Del(buf);
                buf = copied;

                PadStr_Del(obj->command);
                obj->command = PadMem_Move(buf);
                buf = PadStr_New();

                if (!PadCmdline_MoveBack(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back command object");
                    PadStr_Del(buf);
                    return NULL;
                }
                obj = NULL;

                // move back pipe object
                obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__PIPE);

                if (!PadCmdline_MoveBack(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back PIPE object");
                    PadStr_Del(buf);
                    return NULL;
                }
            } else if (*p == '&' && *(p+1) == '&') {
                ++p;

                if (!PadStr_Len(buf)) {
                    snprintf(self->what, sizeof self->what, "invalid command line (2)");
                    PadStr_Del(buf);
                    return NULL;
                }

                // move back cmd object
                PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);
                if (!PadCmdlineObj_Parse(obj, PadStr_Getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    PadStr_Del(buf);
                    return NULL;
                }

                PadStr *copied = PadStr_Strip(buf, " ");
                PadStr_Del(buf);
                buf = copied;

                PadStr_Del(obj->command);
                obj->command = buf;
                buf = PadStr_New();

                if (!PadCmdline_MoveBack(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back command object");
                    PadStr_Del(buf);
                    return NULL;
                }
                obj = NULL;

                // move back and object
                obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__AND);

                if (!PadCmdline_MoveBack(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back AND object");
                    PadStr_Del(buf);
                    return NULL;
                }
            } else if (*p == '>') {
                if (!PadStr_Len(buf)) {
                    snprintf(self->what, sizeof self->what, "invalid command line (3)");
                    PadStr_Del(buf);
                    return NULL;
                }

                // move back cmd object
                PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);
                if (!PadCmdlineObj_Parse(obj, PadStr_Getc(buf))) {
                    snprintf(self->what, sizeof self->what, "failed to parse mini command line");
                    PadStr_Del(buf);
                    return NULL;
                }

                PadStr *copied = PadStr_Strip(buf, " ");
                PadStr_Del(buf);
                buf = copied;

                PadStr_Del(obj->command);
                obj->command = buf;
                buf = PadStr_New();

                if (!PadCmdline_MoveBack(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back command object");
                    PadStr_Del(buf);
                    return NULL;
                }
                obj = NULL;

                // move back and object
                obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__REDIRECT);

                if (!PadCmdline_MoveBack(self, obj)) {
                    snprintf(self->what, sizeof self->what, "failed to move back REDIRECT object");
                    PadStr_Del(buf);
                    return NULL;
                }
            } else {
                PadStr_PushBack(buf, *p);
            }
            break;
        case 10: // found "
            if (*p == '\\') {
                ++p;
                if (*p != '\0') {
                    PadStr_PushBack(buf, *p);
                } else {
                    --p;
                }
            } else if (*p == '"') {
                PadStr_PushBack(buf, *p);
                m = 0;
            } else {
                PadStr_PushBack(buf, *p);
            }
            break;
        }
    }

    if (PadStr_Len(buf)) {
        // move back cmd object
        PadCmdlineObj *obj = PadCmdlineObj_New(PAD_CMDLINE_OBJ_TYPE__CMD);
        if (!PadCmdlineObj_Parse(obj, PadStr_Getc(buf))) {
            snprintf(self->what, sizeof self->what, "failed to parse mini command line (2)");
            PadCmdlineObj_Del(obj);
            PadStr_Del(buf);
            return NULL;
        }

        PadStr *copied = PadStr_Strip(buf, " ");
        PadStr_Del(buf);
        buf = copied;
        
        PadStr_Del(obj->command);
        obj->command = PadMem_Move(buf);
        buf = NULL;

        if (!PadCmdline_MoveBack(self, obj)) {
            snprintf(self->what, sizeof self->what, "failed to move back command object (2)");
            PadCmdlineObj_Del(obj);
            PadStr_Del(buf);
            return NULL;
        }
    }

    PadStr_Del(buf);
    return self;
}
