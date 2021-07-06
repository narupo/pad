/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <pad/lib/string.h>

/*******************
* string structure *
*******************/

struct PadStr {
    int length;
    int capacity;
    PadStrType *buffer;
};

/*************
* str macros *
*************/

#define NCHAR (sizeof(PadStrType))
#define NIL ('\0')

/**************************
* str constant variabless *
**************************/

enum {
    NCAPACITY = 4,
};

/*********************
* str delete and new *
*********************/

void
PadStr_Del(PadStr *self) {
    if (!self) {
        return;
    }

    free(self->buffer);
    free(self);
}

PadStrType *
PadStr_EscDel(PadStr *self) {
    if (!self) {
        return NULL;
    }

    PadStrType *buf = self->buffer;
    free(self);
    return buf;
}

PadStr *
PadStr_New(void) {
    PadStr *self = calloc(1, sizeof(PadStr));
    if (!self) {
        return NULL;
    }

    self->length = 0;
    self->capacity = NCAPACITY;
    self->buffer = calloc(self->capacity + 1, NCHAR);
    if (!self->buffer) {
        free(self);
        return NULL;
    }

    self->buffer[self->length] = NIL;

    return self;
}

PadStr *
PadStr_NewCStr(const PadStrType *str) {
    if (!str) {
        return NULL;
    }

    PadStr *self = PadStr_New();
    PadStr_Set(self, str);
    return self;
}

PadStr *
PadStr_DeepCopy(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    PadStr *self = calloc(1, sizeof(PadStr));
    if (!self) {
        return NULL;
    }

    self->length = other->length;
    self->capacity = other->capacity;
    self->buffer = calloc(self->capacity + 1, NCHAR);
    if (!self->buffer) {
        free(self);
        return NULL;
    }

    for (int i = 0; i < self->length; ++i) {
        self->buffer[i] = other->buffer[i];
    }
    self->buffer[self->length] = NIL;

    return self;
}

PadStr *
PadStr_ShallowCopy(const PadStr *other) {
    return PadStr_DeepCopy(other);
}

/*************
* str getter *
*************/

int32_t
PadStr_Len(const PadStr *self) {
    if (!self) {
        return -1;
    }
    return self->length;
}

int32_t
PadStr_Capa(const PadStr *self) {
    if (!self) {
        return -1;
    }
    return self->capacity;
}

const PadStrType *
PadStr_Getc(const PadStr *self) {
    if (!self) {
        return NULL;
    }
    return self->buffer;
}

int32_t
PadStr_Empty(const PadStr *self) {
    if (!self) {
        return 0;
    }
    return self->length == 0;
}

/*************
* str setter *
*************/

void
PadStr_Clear(PadStr *self) {
    if (!self) {
        return;
    }

    self->length = 0;
    self->buffer[self->length] = NIL;
}

PadStr *
PadStr_Set(PadStr *self, const PadStrType *src) {
    if (!self || !src) {
        return NULL;
    }

    int srclen = strlen(src);
    if (srclen >= self->length) {
        if (!PadStr_Resize(self, srclen)) {
            return NULL;
        }
    }
    self->length = srclen;

    for (int i = 0; i < srclen; ++i) {
        self->buffer[i] = src[i];
    }
    self->buffer[srclen] = NIL;

    return self;
}

PadStr *
PadStr_Resize(PadStr *self, int32_t newcapa) {
    if (!self) {
        return NULL;
    }

    if (newcapa < 0) {
        newcapa = 0;
    }

    PadStrType *tmp = realloc(self->buffer, newcapa*NCHAR + NCHAR); // +NCHAR for final nil
    if (!tmp) {
        PadStr_Del(self);
        return NULL;
    }

    self->buffer = tmp;
    self->capacity = newcapa;
    if (newcapa < self->length) {
        self->length = newcapa;
        self->buffer[self->length] = NIL;
    }

    return self;
}

PadStr *
PadStr_PushBack(PadStr *self, PadStrType ch) {
    if (!self || ch == NIL) {
        return NULL;
    }

    if (self->length >= self->capacity-1) {
        if (!PadStr_Resize(self, self->length*2)) {
            return NULL;
        }
    }

    self->buffer[self->length++] = ch;
    self->buffer[self->length] = NIL;

    return self;
}

PadStrType
PadStr_PopBack(PadStr *self) {
    if (!self) {
        return NIL;
    }

    if (self->length > 0) {
        PadStrType ret = self->buffer[--self->length];
        self->buffer[self->length] = NIL;
        return ret;
    }

    return NIL;
}

PadStr *
PadStr_PushFront(PadStr *self, PadStrType ch) {
    if (!self || ch == NIL) {
        return NULL;
    }

    if (self->length >= self->capacity-1) {
        if (!PadStr_Resize(self, self->length*2)) {
            return NULL;
        }
    }

    for (int32_t i = self->length; i > 0; --i) {
        self->buffer[i] = self->buffer[i-1];
    }

    self->buffer[0] = ch;
    self->buffer[++self->length] = NIL;
    return self;
}

PadStrType
PadStr_PopFront(PadStr *self) {
    if (!self || self->length == 0) {
        return NIL;
    }

    PadStrType ret = self->buffer[0];

    for (int32_t i = 0; i < self->length-1; ++i) {
        self->buffer[i] = self->buffer[i+1];
    }

    --self->length;
    self->buffer[self->length] = NIL;

    return ret;
}

PadStr *
PadStr_App(PadStr *self, const PadStrType *src) {
    if (!self || !src) {
        return NULL;
    }

    int32_t srclen = strlen(src);

    if (self->length + srclen >= self->capacity-1) {
        if (!PadStr_Resize(self, (self->length + srclen) * 2)) {
            return NULL;
        }
    }

    for (const PadStrType *sp = src; *sp; ++sp) {
        self->buffer[self->length++] = *sp;
    }
    self->buffer[self->length] = NIL;

    return self;
}

PadStr *
PadStr_AppStream(PadStr *self, FILE *fin) {
    if (!self || !fin) {
        return NULL;
    }

    for (int32_t ch; (ch = fgetc(fin)) != EOF; ) {
        if (!PadStr_PushBack(self, ch)) {
            return NULL;
        }
    }

    return self;
}

PadStr *
PadStr_AppOther(PadStr *self, const PadStr *_other) {
    if (!self || !_other) {
        return NULL;
    }

    PadStr *other = PadStr_DeepCopy(_other);
    PadStr *ret = NULL;

    if (self == other) {
        PadStrType *buf = PadCStr_Dup(self->buffer);
        if (!buf) {
            PadStr_Del(other);
            return ret;
        }
        ret = PadStr_App(self, buf);
        free(buf);
    } else {
        ret = PadStr_App(self, other->buffer);
    }

    PadStr_Del(other);
    return ret;
}

PadStr *
PadStr_AppFmt(PadStr *self, PadStrType *buf, int32_t nbuf, const PadStrType *fmt, ...) {
    if (!self || !buf || !fmt || nbuf == 0) {
        return NULL;
    }

    va_list args;
    va_start(args, fmt);
    int32_t buflen = vsnprintf(buf, nbuf, fmt, args);
    va_end(args);

    for (int32_t i = 0; i < buflen; ++i) {
        if (!PadStr_PushBack(self, buf[i])) {
            return NULL;
        }
    }

    return self;
}

static PadStr *
_PadStr_RStrip(PadStr *self, const PadStrType *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (int32_t i = self->length-1; i > 0; --i) {
        if (strchr(rems, self->buffer[i])) {
            self->buffer[i] = NIL;
        } else {
            break;
        }
    }

    return self;
}

PadStr *
PadStr_RStrip(const PadStr *other, const PadStrType *rems) {
    if (!other || !rems) {
        return NULL;
    }

    PadStr *dst = PadStr_DeepCopy(other);
    if (!_PadStr_RStrip(dst, rems)) {
        PadStr_Del(dst);
        return NULL;
    }

    return dst;
}

static PadStr *
_PadStr_LStrip(PadStr *self, const PadStrType *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (; self->length; ) {
        if (strchr(rems, self->buffer[0])) {
            PadStr_PopFront(self);
        } else {
            break;
        }
    }

    return self;
}

PadStr *
PadStr_LStrip(const PadStr *other, const PadStrType *rems) {
    if (!other || !rems) {
        return NULL;
    }

    PadStr *dst = PadStr_DeepCopy(other);
    if (!_PadStr_LStrip(dst, rems)) {
        PadStr_Del(dst);
        return NULL;
    }

    return dst;
}

PadStr *
PadStr_Strip(const PadStr *other, const PadStrType *rems) {
    if (!other || !rems) {
        return NULL;
    }

    PadStr *dst = PadStr_DeepCopy(other);

    if (!_PadStr_RStrip(dst, rems)) {
        PadStr_Del(dst);
        return NULL;
    }

    if (!_PadStr_LStrip(dst, rems)) {
        PadStr_Del(dst);
        return NULL;
    }

    return dst;
}

/****************
* str algorithm *
****************/

#define MAX(a, b) (a > b ? a : b)
/**
 * Boyer-Moore search at first
 *
 * @param[in]  tex	  Target string
 * @param[in]  texlen Target length
 * @param[in]  pat	  Pattern string
 * @param[in]  patlen Pattern length
 * @return		  Success to pointer to found position in target string
 * @return		  Failed to NULL
 */
static const PadStrType *
bmfind(
    const PadStrType *restrict tex,
    int32_t texlen,
    const PadStrType *restrict pat,
    int32_t patlen
) {
    int32_t const max = CHAR_MAX+1;
    ssize_t texpos = 0;
    ssize_t patpos = 0;
    int32_t table[max];

    if (texlen < patlen || patlen <= 0) {
        return NULL;
    }

    for (int32_t i = 0; i < max; ++i) {
        table[i] = patlen;
    }

    for (int32_t i = 0; i < patlen; ++i) {
        table[ (int32_t)pat[i] ] = patlen-i-1;
    }

    texpos = patlen-1;

    while (texpos <= texlen) {
        int32_t curpos = texpos;
        patpos = patlen-1;
        while (tex[texpos] == pat[patpos]) {
            if (patpos <= 0) {
                return tex + texpos;
            }
            --patpos;
            --texpos;
        }
        int32_t index = (int32_t)tex[texpos];
        texpos = MAX(curpos+1, texpos + table[ index ]);
    }
    return NULL;
}
#undef MAX

const PadStrType *
PadStr_Findc(const PadStr *self, const PadStrType *target) {
    if (!self || !target) {
        return NULL;
    }

    return bmfind(self->buffer, self->length, target, strlen(target));
}

PadStr *
PadStr_Lower(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    PadStr *self = PadStr_DeepCopy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        PadStrType ch = self->buffer[i];
        if (isupper(ch)) {
            self->buffer[i] = tolower(ch);
        }
    }

    return self;
}

PadStr *
PadStr_Upper(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    PadStr *self = PadStr_DeepCopy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        PadStrType ch = self->buffer[i];
        if (islower(ch)) {
            self->buffer[i] = toupper(ch);
        }
    }

    return self;
}

PadStr *
PadStr_Capi(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    PadStr *self = PadStr_DeepCopy(other);
    if (self->length) {
        PadStrType ch = self->buffer[0];
        if (islower(ch)) {
            self->buffer[0] = toupper(ch);
        }
    }

    return self;
}

PadStr *
PadStr_Snake(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    const PadStrType *p = PadStr_Getc(other);
    PadStr *self = PadStr_New();

    for (; *p; ++p) {
        switch (m) {
        case 0: // first
            if (*p == '-' || *p == '_') {
                m = 10;
            } else {
                PadStr_PushBack(self, tolower(*p));
                m = 20;
            }
            break;
        case 10: // found '-' or '_'
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                PadStr_PushBack(self, tolower(*p));
                m = 20;
            }
            break;
        case 20: // found normal character
            if (isupper(*p)) {
                PadStr_PushBack(self, '_');
                PadStr_PushBack(self, tolower(*p));
            } else if (*p == '-' || *p == '_') {
                PadStr_PushBack(self, '_');
                m = 10;
            } else {
                PadStr_PushBack(self, *p);
            }
            break;
        }
    }

    return self;
}

PadStr *
PadStr_Camel(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    PadStr *self = PadStr_New();
    const PadStrType *p = PadStr_Getc(other);

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == '-' || *p == '_') {
                m = 10;
            } else {
                PadStr_PushBack(self, tolower(*p));
                m = 20;
            }
            break;
        case 10:  // found '-' or '_' first
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                PadStr_PushBack(self, tolower(*p));
                m = 20;
            }
            break;
        case 15:  // found '-' or '_' second
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                PadStr_PushBack(self, toupper(*p));
                m = 30;
            }
            break;
        case 20:  // readed lower character
            if (*p == '-' || *p == '_') {
                m = 15;
            } else if (isupper(*p)) {
                PadStr_PushBack(self, *p);
                m = 30;
            } else {
                PadStr_PushBack(self, *p);
            }
            break;
        case 30:  // readed upper character
            if (*p == '-' || *p == '_') {
                m = 15;
            } else if (isupper(*p)) {
                PadStr_PushBack(self, *p);
            } else {
                PadStr_PushBack(self, *p);
                m = 20;
            }
            break;
        }
    }

    return self;
}

PadStr *
PadStr_Hacker(const PadStr *other) {
    if (!other) {
        return NULL;
    }

    PadStr *self = PadStr_New();
    const PadStrType *p = PadStr_Getc(other);
    int m = 0;

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == '-' || *p == '_') {
                m = 100;
            } else if (isupper(*p)) {
                PadStr_PushBack(self, tolower(*p));
            } else if (islower(*p)) {
                PadStr_PushBack(self, *p);
            } else if (isdigit(*p)) {
                PadStr_PushBack(self, *p);
            } else {
                PadStr_PushBack(self, *p);
            }
            break;
        case 100:  // skip '-' or '_'
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                --p;
                m = 0;
            }
            break;
        }
    }

    return self;
}

PadStr *
PadStr_Mul(const PadStr *self, int32_t n) {
    if (!self) {
        return NULL;
    }
    
    PadStr *buf = PadStr_New();

    for (int32_t i = 0; i < n; ++i) {
        PadStr_App(buf, self->buffer);
    }

    return buf;
}

PadStr *
PadStr_Indent(const PadStr *other, int32_t ch, int32_t n, int32_t tabsize) {
    if (!other || ch < 0 || n < 0 || tabsize < 0) {
        return NULL;
    }

    if (tabsize == 0) {
        tabsize = 1;
    }

    int32_t valsz = n * tabsize;
    char value[valsz + 1];
    if (ch == ' ') {
        memset(value, ch, valsz);
        value[valsz] = '\0';
    } else {
        memset(value, ch, valsz);
        value[n] = '\0';
    }

    PadStr *self = PadStr_New();
    const char *p = PadStr_Getc(other);

    PadStr_App(self, value);
    for (; *p; ++p) {
        if (*p == '\r' && *(p + 1) == '\n') {
            ++p;
            PadStr_App(self, "\r\n");
            if (*(p + 1)) {
                PadStr_App(self, value);
            }
        } else if (*p == '\r' || *p == '\n') {
            PadStr_PushBack(self, *p);
            if (*(p + 1)) {
                PadStr_App(self, value);
            }
        } else {
            PadStr_PushBack(self, *p);
        }
    }

    return self;
}

/**************
* str cleanup *
**************/

#undef NCHAR
#undef NIL
