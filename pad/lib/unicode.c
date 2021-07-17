#include <pad/lib/unicode.h>

#define NIL PAD_UNI__CH('\0')

/**********
* utility *
**********/

int32_t
PadChar32_Len(const char32_t *str) {
    int32_t len = 0;
    for (const char32_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

int32_t
PadChar16_Len(const char16_t *str) {
    int32_t len = 0;
    for (const char16_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

char32_t *
PadChar32_Dup(const char32_t *str) {
    int32_t len = PadChar32_Len(str);
    int32_t byte = sizeof(char32_t);

    char32_t *dst = calloc(len + 1, byte);
    if (!dst) {
        return NULL;
    }

    for (int32_t i = 0; i < len; ++i) {
        dst[i] = str[i];
    }
    dst[len] = NIL;

    return dst;
}

char16_t *
PadChar16_Dup(const char16_t *str) {
    int32_t len = PadChar16_Len(str);
    int32_t byte = sizeof(char16_t);

    char16_t *dst = calloc(len + 1, byte);
    if (!dst) {
        return NULL;
    }

    for (int32_t i = 0; i < len; ++i) {
        dst[i] = str[i];
    }
    dst[len] = NIL;

    return dst;
}

bool
PadChar32_IsAlpha(char32_t ch) {
    return (ch >= 65 && ch <= 90) ||
           (ch >= 97 && ch <= 122);
}

bool
PadChar16_IsAlpha(char16_t ch) {
    return (ch >= 65 && ch <= 90) ||
           (ch >= 97 && ch <= 122);
}

bool
PadChar32_IsLower(char32_t ch) {
    return ch >= 97 && ch <= 122;
}

bool
PadChar16_IsLower(char16_t ch) {
    return ch >= 97 && ch <= 122;
}

bool
PadChar32_IsUpper(char32_t ch) {
    return ch >= 65 && ch <= 90;
}

bool
PadChar16_IsUpper(char16_t ch) {
    return ch >= 65 && ch <= 90;
}

char32_t
PadChar32_ToLower(char32_t ch) {
    if (PadChar32_IsUpper(ch)) {
        return ch + 32;
    }

    return ch;
}

char16_t
PadChar16_ToLower(char16_t ch) {
    if (PadChar16_IsUpper(ch)) {
        return ch + 32;
    }

    return ch;
}

char32_t
PadChar32_ToUpper(char32_t ch) {
    if (PadChar32_IsLower(ch)) {
        return ch - 32;
    }

    return ch;
}

char16_t
PadChar16_ToUpper(char16_t ch) {
    if (PadChar16_IsLower(ch)) {
        return ch - 32;
    }

    return ch;
}

bool
PadChar32_IsDigit(char32_t ch) {
    return ch >= 48 && ch <= 57;
}

bool
PadChar16_IsDigit(char16_t ch) {
    return ch >= 48 && ch <= 57;
}

int32_t
PadChar32_StrCmp(const char32_t *s1, const char32_t *s2) {
    if (!s1 || !s2) {
        return -1;
    }
    if (PadChar32_Len(s1) != PadChar32_Len(s2)) {
        return -1;
    }

    for (const char32_t *p = s1, *q = s2; *p && *q; ++p, ++q) {
        if (*p != *q) {
            return *p - *q;
        }
    }

    return 0;
}

int32_t
PadChar16_StrCmp(const char16_t *s1, const char16_t *s2) {
    if (!s1 || !s2) {
        return -1;
    }
    if (PadChar16_Len(s1) != PadChar16_Len(s2)) {
        return -1;
    }

    for (const char16_t *p = s1, *q = s2; *p && *q; ++p, ++q) {
        if (*p != *q) {
            return *p - *q;
        }
    }

    return 0;
}

int32_t
PadChar32_StrNCmp(const char32_t *s1, const char32_t *s2, int32_t n) {
    if (!s1 || !s2) {
        return -1;
    }
    int32_t s1len = PadU_Len(s1);
    int32_t s2len = PadU_Len(s2);

    for (int32_t i = 0; i < n && i < s1len && i < s2len; i++) {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
    }

    return 0;
}

int32_t
PadChar16_StrNCmp(const char16_t *s1, const char16_t *s2, int32_t n) {
    if (!s1 || !s2) {
        return -1;
    }
    int32_t s1len = PadU_Len(s1);
    int32_t s2len = PadU_Len(s2);

    for (int32_t i = 0; i < n && i < s1len && i < s2len; i++) {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
    }

    return 0;
}

bool
PadChar16_IsSpace(char16_t ch) {
    return ch == u'\n' || ch == u'\t' || ch == u' ';
}

bool
PadChar32_IsSpace(char32_t ch) {
    return ch == U'\n' || ch == U'\t' || ch == U' ';
}

/**********
* unicode *
**********/

struct PadUni {
    PadUniType *buffer;
    int32_t length;
    int32_t capacity;
    char *mb;
};

void
PadUni_Del(PadUni *self) {
    if (!self) {
        return;
    }

    free(self->buffer);
    free(self->mb);
    free(self);
}

PadUniType *
PadUni_EscDel(PadUni *self) {
    if (!self) {
        return NULL;
    }

    PadUniType *esc = self->buffer;
    free(self->mb);
    free(self);

    return esc;
}

PadUni *
PadUni_New(void) {
    PadUni *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->length = 0;
    self->capacity = PAD_UNI__INIT_CAPA;
    self->buffer = calloc(self->capacity + 1, sizeof(PadUniType));
    if (!self->buffer) {
        free(self);
        return NULL;
    }

    return self;
}

PadUni *
PadUni_NewCStr(const char *str) {
    PadUni *self = PadUni_New();
    if (!self) {
        return NULL;
    }
    
    return PadUni_SetMB(self, str);
}

PadUni *
PadUni_Resize(PadUni *self, int32_t newcapa) {
    if (!self || newcapa < 0) {
        return NULL;
    }

    int32_t byte = sizeof(PadUniType);
    int32_t size = newcapa * byte + byte;
    PadUniType *tmp = realloc(self->buffer, size);
    if (!tmp) {
        return NULL;
    }
    self->buffer = tmp;

    if (newcapa < self->length) {
        self->length = newcapa;
    }

    self->capacity = newcapa;
    self->buffer[self->length] = NIL;

    return self;
}

int32_t
PadUni_Len(const PadUni *self) {
    if (!self) {
        return -1;
    }

    return self->length;
}

int32_t
PadUni_Capa(const PadUni *self) {
    if (!self) {
        return -1;
    }

    return self->capacity;
}

PadUniType *
PadUni_Get(PadUni *self) {
    if (!self) {
        return NULL;
    }

    return self->buffer;
}

const PadUniType *
PadUni_Getc(const PadUni *self) {
    if (!self) {
        return NULL;
    }

    return self->buffer;
}

bool
PadUni_Empty(const PadUni *self) {
    if (!self) {
        return 0;
    }

    return self->length == 0;
}

void
PadUni_Clear(PadUni *self) {
    if (!self) {
        return;
    }

    self->length = 0;
    self->buffer[self->length] = NIL;
}

PadUni *
PadUni_Set(PadUni *self, const PadUniType *src) {
    if (!self || !src) {
        return NULL;
    }

    int srclen = PadU_Len(src);
    if (srclen >= self->length) {
        if (!PadUni_Resize(self, srclen)) {
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

PadUni *
PadUni_PushBack(PadUni *self, PadUniType ch) {
    if (!self) {
        return NULL;
    }
    if (ch == PAD_UNI__CH('\0')) {
        return NULL;
    }

    if (self->length >= self->capacity) {
        if (!PadUni_Resize(self, self->capacity * 2)) {
            return NULL;
        }
    }

    self->buffer[self->length++] = ch;
    self->buffer[self->length] = NIL;

    return self;
}

PadUniType
PadUni_PopBack(PadUni *self) {
    if (!self) {
        return NIL;
    }

    if (self->length > 0) {
        PadUniType ret = self->buffer[--self->length];
        self->buffer[self->length] = NIL;
        return ret;
    }

    return NIL;
}

PadUni *
PadUni_PushFront(PadUni *self, PadUniType ch) {
    if (!self || ch == NIL) {
        return NULL;
    }

    if (self->length >= self->capacity - 1) {
        if (!PadUni_Resize(self, self->length * 2)) {
            return NULL;
        }
    }

    for (int32_t i = self->length; i > 0; --i) {
        self->buffer[i] = self->buffer[i - 1];
    }

    self->buffer[0] = ch;
    self->buffer[++self->length] = NIL;
    return self;
}

PadUniType
PadUni_PopFront(PadUni *self) {
    if (!self || self->length == 0) {
        return NIL;
    }

    PadUniType ret = self->buffer[0];

    for (int32_t i = 0; i < self->length - 1; ++i) {
        self->buffer[i] = self->buffer[i + 1];
    }

    --self->length;
    self->buffer[self->length] = NIL;

    return ret;
}

PadUni *
PadUni_App(PadUni *self, const PadUniType *src) {
    if (!self || !src) {
        return NULL;
    }

    int32_t srclen = PadU_Len(src);
    int32_t totallen = self->length + srclen;

    if (totallen >= self->capacity - 1) {
        int32_t newcapa = totallen * 2;
        if (!PadUni_Resize(self, newcapa)) {
            return NULL;
        }
    }

    for (const PadUniType *sp = src; *sp; ++sp) {
        self->buffer[self->length++] = *sp;
    }
    self->buffer[self->length] = NIL;

    return self;
}

PadUni *
PadUni_AppStream(PadUni *self, FILE *fin) {
    if (!self || !fin) {
        return NULL;
    }

    for (int32_t ch; (ch = fgetc(fin)) != EOF; ) {
        if (!PadUni_PushBack(self, ch)) {
            return NULL;
        }
    }

    return self;
}

PadUni *
PadUni_DeepCopy(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    PadUni *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    int32_t byte = sizeof(PadUniType);
    self->capacity = other->capacity;
    self->buffer = PadMem_Calloc(self->capacity + 1, byte);
    if (!self->buffer) {
        free(self);
        return NULL;
    }

    for (self->length = 0; self->length < other->length; ++self->length) {
        self->buffer[self->length] = other->buffer[self->length];
    }
    self->buffer[self->length] = NIL;

    // not need copy mb. this is temporary buffer
    //
    // self->mb = PadCStr_EDup(other->mb);

    return self;
}

PadUni *
PadUni_ShallowCopy(const PadUni *other) {
    return PadUni_DeepCopy(other);
}

PadUni *
PadUni_AppOther(PadUni *self, const PadUni *_other) {
    if (!self || !_other) {
        return NULL;
    }

    PadUni *other = PadUni_DeepCopy(_other);
    PadUni *ret = NULL;

    if (self == other) {
        PadUniType *buf = PadU_StrDup(self->buffer);
        if (!buf) {
            PadUni_Del(other);
            return ret;
        }
        ret = PadUni_App(self, buf);
        free(buf);
    } else {
        ret = PadUni_App(self, other->buffer);
    }

    PadUni_Del(other);
    return ret;
}

PadUni *
PadUni_AppFmt(PadUni *self, char *buf, int32_t nbuf, const char *fmt, ...) {
    if (!self || !buf || !fmt || nbuf == 0) {
        return NULL;
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, nbuf, fmt, args);
    va_end(args);

    PadUni *tail = PadUni_New();
    PadUni_SetMB(tail, buf);

    for (const PadUniType *p = PadUni_Getc(tail); *p; ++p) {
        PadUni_PushBack(self, *p);
    }

    PadUni_Del(tail);
    return self;
}

char *
PadUni_ToMB(const PadUni *self) {
    if (!self) {
        return NULL;
    }

    mbstate_t mbstate = {0};
    PadStr *buf = PadStr_New();

    for (int32_t i = 0; i < self->length; ++i) {
        char mb[MB_LEN_MAX + 1];
#if defined(PAD_UNI__CHAR32)
        size_t result = c32rtomb(mb, self->buffer[i], &mbstate);
#elif defined(PAD_UNI__CH16)
        size_t result = c16rtomb(mb, self->buffer[i], &mbstate);
#endif
        if (result == -1) {
            return NULL;
        }

        mb[result] = '\0';

        for (const char *p = mb; *p; ++p) {
            PadStr_PushBack(buf, *p);
        }
    }

    return PadStr_EscDel(buf);
}

PadUni *
PadUni_SetMB(PadUni *self, const char *mb) {
    if (!self || !mb) {
        return NULL;
    }

    int32_t len = strlen(mb);
    const char *end = mb + len;
    mbstate_t mbstate = {0};
    int mbi = 0;

    PadUni_Clear(self);

    for (; mbi < len;) {
        char32_t c32;
        mbstate = (mbstate_t) {0};
        errno = 0;
        const int result = mbrtoc32(&c32, &mb[mbi], end - &mb[mbi], &mbstate);
        if (result > 0) {
            mbi += result;
        } else if (result == 0) {
            // reached null terminator
            break;
        } else if (result == -1) {
            // invalid bytes
            // fprintf(stderr, "PadUni_SetMB: invalid characters\n");
            if (errno) {
                // perror("mbrtoc32");
            }
            return NULL;
        } else if (result == -2) {
            // don't display error messages (i hate these error messages)
            // fprintf(stderr, "PadUni_SetMB: incomplete input characters\n");
            if (errno) {
                // perror("mbrtoc32");
            }
            return NULL;
        } else if (result == -3) {
            // char32_t の文字を構成する残りの部分を得た。
            // マルチバイト文字側のバイトは消費していない
            // fprintf(stderr, "PadUni_SetMB: got -3\n");
        }

        if (!PadUni_PushBack(self, c32)) {
            return NULL;
        }
    }

    return self;
}

static PadUni *
_PadUni_RStrip(PadUni *self, const PadUniType *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (int32_t i = self->length - 1; i > 0; --i) {
        bool found = false;
        for (const PadUniType *p = rems; *p; ++p) {
            if (*p == self->buffer[i]) {
                self->buffer[i] = NIL;
                found = true;
            }
        }
        if (!found) {
            break;
        }
    }

    return self;
}

PadUni *
PadUni_RStrip(const PadUni *other, const PadUniType *rems) {
    if (!other || !rems) {
        return NULL;
    }

    PadUni *dst = PadUni_DeepCopy(other);
    if (!_PadUni_RStrip(dst, rems)) {
        PadUni_Del(dst);
        return NULL;
    }

    return dst;
}

static PadUni *
_PadUni_LStrip(PadUni *self, const PadUniType *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (; self->length; ) {
        bool found = false;
        for (const PadUniType *p = rems; *p; ++p) {
            if (*p == self->buffer[0]) {
                PadUni_PopFront(self);
                found = true;
            }
        }
        if (!found) {
            break;
        }
    }

    return self;
}

PadUni *
PadUni_LStrip(const PadUni *other, const PadUniType *rems) {
    if (!other || !rems) {
        return NULL;
    }

    PadUni *dst = PadUni_DeepCopy(other);
    if (!_PadUni_LStrip(dst, rems)) {
        PadUni_Del(dst);
        return NULL;
    }

    return dst;
}

PadUni *
PadUni_Strip(const PadUni *other, const PadUniType *rems) {
    if (!other || !rems) {
        return NULL;
    }

    PadUni *dst = PadUni_DeepCopy(other);

    if (!_PadUni_RStrip(dst, rems)) {
        PadUni_Del(dst);
        return NULL;
    }

    if (!_PadUni_LStrip(dst, rems)) {
        PadUni_Del(dst);
        return NULL;
    }

    return dst;
}

const char *
PadUni_GetcMB(PadUni *self) {
    char *mb = PadUni_ToMB(self);
    if (!mb) {
        return NULL;
    }

    if (self->mb) {
        free(self->mb);
    }
    self->mb = mb;

    return self->mb;
}

static char *
conv_to_mb(PadUni *self, char *mb, PadUniType ch) {
    if (!self || !mb) {
        return NULL;
    }

    size_t result;
    mbstate_t mbstate = {0};

#if defined(PAD_UNI__CHAR32)
    result = c32rtomb(mb, ch, &mbstate);
#elif defined(PAD_UNI__CH16)
    result = c16rtomb(mb, ch, &mbstate);
#endif

    if (result == -1) {
        return NULL;
    }

    mb[result] = '\0';
    return mb;
}

PadUni *
PadUni_Lower(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    PadUni *self = PadUni_DeepCopy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        PadUniType ch = self->buffer[i];
        if (PadU_IsUpper(ch)) {
            self->buffer[i] = PadU_ToLower(ch);
        }
    }

    return self;
}

PadUni *
PadUni_Upper(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    PadUni *self = PadUni_DeepCopy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        PadUniType ch = self->buffer[i];
        if (PadU_IsLower(ch)) {
            self->buffer[i] = PadU_ToUpper(ch);
        }
    }

    return self;
}

PadUni *
PadUni_Capi(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    PadUni *self = PadUni_DeepCopy(other);
    if (self->length) {
        PadUniType ch = self->buffer[0];
        if (PadU_IsLower(ch)) {
            self->buffer[0] = PadU_ToUpper(ch);
        }
    }

    return self;
}

PadUni *
PadUni_Snake(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    const PadUniType *p = PadUni_Getc(other);
    PadUni *self = PadUni_New();

    for (; *p; ++p) {
        switch (m) {
        case 0: // first
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                m = 10;
            } else {
                PadUni_PushBack(self, PadU_ToLower(*p));
                m = 20;
            }
            break;
        case 10: // found '-' or '_'
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                // pass
            } else {
                PadUni_PushBack(self, PadU_ToLower(*p));
                m = 20;
            }
            break;
        case 20: // found normal character
            if (PadU_IsUpper(*p)) {
                PadUni_PushBack(self, PAD_UNI__CH('_'));
                PadUni_PushBack(self, PadU_ToLower(*p));
            } else if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                PadUni_PushBack(self, PAD_UNI__CH('_'));
                m = 10;
            } else {
                PadUni_PushBack(self, *p);
            }
            break;
        }
    }

    return self;
}

PadUni *
PadUni_Camel(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    PadUni *self = PadUni_New();
    const PadUniType *p = PadUni_Getc(other);

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                m = 10;
            } else {
                PadUni_PushBack(self, PadU_ToLower(*p));
                m = 20;
            }
            break;
        case 10:  // found '-' or '_' first
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                // pass
            } else {
                PadUni_PushBack(self, PadU_ToLower(*p));
                m = 20;
            }
            break;
        case 15:  // found '-' or '_' second
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                // pass
            } else {
                PadUni_PushBack(self, PadU_ToUpper(*p));
                m = 30;
            }
            break;
        case 20:  // readed lower character
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                m = 15;
            } else if (PadU_IsUpper(*p)) {
                PadUni_PushBack(self, *p);
                m = 30;
            } else {
                PadUni_PushBack(self, *p);
            }
            break;
        case 30:  // readed upper character
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                m = 15;
            } else if (PadU_IsUpper(*p)) {
                PadUni_PushBack(self, *p);
            } else {
                PadUni_PushBack(self, *p);
                m = 20;
            }
            break;
        }
    }

    return self;
}

PadUni *
PadUni_Hacker(const PadUni *other) {
    if (!other) {
        return NULL;
    }

    PadUni *self = PadUni_New();
    const PadUniType *p = PadUni_Getc(other);
    int m = 0;

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == PAD_UNI__CH('-') || *p == PAD_UNI__CH('_')) {
                m = 100;
            } else if (PadU_IsUpper(*p)) {
                PadUni_PushBack(self, PadU_ToLower(*p));
            } else if (PadU_IsLower(*p)) {
                PadUni_PushBack(self, *p);
            } else if (PadU_IsDigit(*p)) {
                PadUni_PushBack(self, *p);
            } else {
                PadUni_PushBack(self, *p);
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

PadUni *
PadUni_Mul(const PadUni *other, int32_t n) {
    if (!other) {
        return NULL;
    }
    
    PadUni *buf = PadUni_New();

    for (int32_t i = 0; i < n; ++i) {
        PadUni_App(buf, other->buffer);
    }

    return buf;
}

PadUni **
PadUni_Split(const PadUni *other, const PadUniType *sep) {
    if (!other) {
        return NULL;
    }

    int32_t capa = 4;
    int32_t cursize = 0;
    PadUni **arr = PadMem_Calloc(capa + 1, sizeof(PadUni *));
    if (!arr) {
        return NULL;
    }

    PadUni *u = PadUni_New();

#define store(u) \
    if (PadUni_Len(u)) { \
        if (capa >= cursize) { \
            capa *= 2; \
            int32_t nbyte = sizeof(PadUni *); \
            PadUni **tmp = PadMem_Realloc(arr, capa * nbyte + nbyte); \
            if (!tmp) { \
                return NULL; \
            } \
            arr = tmp; \
        } \
        arr[cursize++] = PadMem_Move(u); \
        arr[cursize] = NULL; \
        u = PadUni_New(); \
    } \

    int32_t seplen = PadU_Len(sep);

    for (const PadUniType *p = other->buffer; *p; ) {
        if (!PadU_StrNCmp(p, sep, seplen)) {
            store(u);
            p += seplen;
        } else {
            PadUni_PushBack(u, *p++);
        }
    }

    store(u);

    PadUni_Del(u);
    return arr;
}

bool
PadUni_IsDigit(const PadUni *self) {
    if (!self) {
        return false;
    }

    for (const PadUniType *p = self->buffer; *p; ++p) {
        if (!PadU_IsDigit(*p)) {
            return false;
        }
    }

    return true;
}

bool
PadUni_IsAlpha(const PadUni *self) {
    if (!self) {
        return false;
    }

    for (const PadUniType *p = self->buffer; *p; ++p) {
        if (!PadU_IsAlpha(*p)) {
            return false;
        }
    }

    return true;
}

bool
PadUni_IsSpace(const PadUni *self) {
    if (!self) {
        return false;
    }

    for (const PadUniType *p = self->buffer; *p; ++p) {
        if (!PadU_IsSpace(*p)) {
            return false;
        }
    }

    return true;
}

int
PadUni_Compare(const PadUni *self, const PadUni *other) {
    if (!self || !other) {
        return 0;
    }

    return PadU_StrCmp(self->buffer, other->buffer);
}

#undef NIL
