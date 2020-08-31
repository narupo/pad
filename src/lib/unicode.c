#include <lib/unicode.h>

#define NIL UNI_CHAR('\0')

/**********
* utility *
**********/

int32_t
char32_len(const char32_t *str) {
    int32_t len = 0;
    for (const char32_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

int32_t
char16_len(const char16_t *str) {
    int32_t len = 0;
    for (const char16_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

char32_t *
char32_dup(const char32_t *str) {
    int32_t len = char32_len(str);
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
char16_dup(const char16_t *str) {
    int32_t len = char16_len(str);
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
char32_isalpha(char32_t ch) {
    return (ch >= 65 && ch <= 90) ||
           (ch >= 97 && ch <= 122);
}

bool
char16_isalpha(char16_t ch) {
    return (ch >= 65 && ch <= 90) ||
           (ch >= 97 && ch <= 122);
}

bool
char32_islower(char32_t ch) {
    return ch >= 97 && ch <= 122;
}

bool
char16_islower(char16_t ch) {
    return ch >= 97 && ch <= 122;
}

bool
char32_isupper(char32_t ch) {
    return ch >= 65 && ch <= 90;
}

bool
char16_isupper(char16_t ch) {
    return ch >= 65 && ch <= 90;
}

char32_t
char32_tolower(char32_t ch) {
    if (char32_isupper(ch)) {
        return ch + 32;
    }

    return ch;
}

char16_t
char16_tolower(char16_t ch) {
    if (char16_isupper(ch)) {
        return ch + 32;
    }

    return ch;
}

char32_t
char32_toupper(char32_t ch) {
    if (char32_islower(ch)) {
        return ch - 32;
    }

    return ch;
}

char16_t
char16_toupper(char16_t ch) {
    if (char16_islower(ch)) {
        return ch - 32;
    }

    return ch;
}

bool
char32_isdigit(char32_t ch) {
    return ch >= 48 && ch <= 57;
}

bool
char16_isdigit(char16_t ch) {
    return ch >= 48 && ch <= 57;
}

int32_t
char32_strcmp(const char32_t *s1, const char32_t *s2) {
    for (const char32_t *p = s1, *q = s2; *p && *q; ++p, ++q) {
        if (*p != *q) {
            return *p - *q;
        }
    }

    return 0;
}

int32_t
char16_strcmp(const char16_t *s1, const char16_t *s2) {
    for (const char16_t *p = s1, *q = s2; *p && *q; ++p, ++q) {
        if (*p != *q) {
            return *p - *q;
        }
    }

    return 0;
}

/**********
* unicode *
**********/

struct unicode {
    unicode_type_t *buffer;
    int32_t length;
    int32_t capacity;
    char *mb;
};

void
uni_del(unicode_t *self) {
    if (!self) {
        return;
    }

    free(self->buffer);
    free(self->mb);
    free(self);
}

unicode_type_t *
uni_esc_del(unicode_t *self) {
    if (!self) {
        return NULL;
    }

    unicode_type_t *esc = self->buffer;
    free(self->mb);
    free(self);

    return esc;
}

unicode_t *
uni_new(void) {
    unicode_t *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->length = 0;
    self->capacity = UNI_INIT_CAPA;
    self->buffer = calloc(self->capacity + 1, sizeof(unicode_type_t));
    if (!self->buffer) {
        free(self);
        return NULL;
    }

    return self;
}

unicode_t *
uni_resize(unicode_t *self, int32_t newcapa) {
    if (!self || newcapa < 0) {
        return NULL;
    }

    int32_t byte = sizeof(unicode_type_t);
    int32_t size = newcapa * byte + byte;
    unicode_type_t *tmp = realloc(self->buffer, size);
    if (!tmp) {
        return NULL;
    }
    self->buffer = tmp;

    if (newcapa < self->length) {
        self->length = newcapa;
        self->buffer[self->length] = NIL;
    }

    self->capacity = newcapa;

    return self;
}

int32_t
uni_len(const unicode_t *self) {
    if (!self) {
        return -1;
    }

    return self->length;
}

int32_t
uni_capa(const unicode_t *self) {
    if (!self) {
        return -1;
    }

    return self->capacity;
}

unicode_type_t *
uni_get(unicode_t *self) {
    if (!self) {
        return NULL;
    }

    return self->buffer;
}

const unicode_type_t *
uni_getc(const unicode_t *self) {
    if (!self) {
        return NULL;
    }

    return self->buffer;
}

int32_t
uni_empty(const unicode_t *self) {
    if (!self) {
        return 0;
    }

    return self->length == 0;
}

void
uni_clear(unicode_t *self) {
    if (!self) {
        return;
    }

    self->length = 0;
    self->buffer[self->length] = NIL;
}

unicode_t *
uni_set(unicode_t *self, const unicode_type_t *src) {
    if (!self || !src) {
        return NULL;
    }

    int srclen = u_len(src);
    if (srclen >= self->length) {
        if (!uni_resize(self, srclen)) {
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

unicode_t *
uni_pushb(unicode_t *self, unicode_type_t ch) {
    if (!self) {
        return NULL;
    }
    if (ch == UNI_CHAR('\0')) {
        return NULL;
    }

    if (self->length >= self->capacity) {
        if (!uni_resize(self, self->capacity * 2)) {
            return NULL;
        }
    }

    self->buffer[self->length++] = ch;
    self->buffer[self->length] = NIL;

    return self;
}

unicode_type_t
uni_popb(unicode_t *self) {
    if (!self) {
        return NIL;
    }

    if (self->length > 0) {
        unicode_type_t ret = self->buffer[--self->length];
        self->buffer[self->length] = NIL;
        return ret;
    }

    return NIL;
}

unicode_t *
uni_pushf(unicode_t *self, unicode_type_t ch) {
    if (!self || ch == NIL) {
        return NULL;
    }

    if (self->length >= self->capacity - 1) {
        if (!uni_resize(self, self->length * 2)) {
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

unicode_type_t
uni_popf(unicode_t *self) {
    if (!self || self->length == 0) {
        return NIL;
    }

    unicode_type_t ret = self->buffer[0];

    for (int32_t i = 0; i < self->length - 1; ++i) {
        self->buffer[i] = self->buffer[i + 1];
    }

    --self->length;
    self->buffer[self->length] = NIL;

    return ret;
}

unicode_t *
uni_app(unicode_t *self, const unicode_type_t *src) {
    if (!self || !src) {
        return NULL;
    }

    int32_t srclen = u_len(src);
    int32_t totallen = self->length + srclen;

    if (totallen >= self->capacity - 1) {
        int32_t newcapa = totallen * 2;
        if (!uni_resize(self, newcapa)) {
            return NULL;
        }
    }

    for (const unicode_type_t *sp = src; *sp; ++sp) {
        self->buffer[self->length++] = *sp;
    }
    self->buffer[self->length] = NIL;

    return self;
}

unicode_t *
uni_app_stream(unicode_t *self, FILE *fin) {
    if (!self || !fin) {
        return NULL;
    }

    for (int32_t ch; (ch = fgetc(fin)) != EOF; ) {
        if (!uni_pushb(self, ch)) {
            return NULL;
        }
    }

    return self;
}

unicode_t *
uni_deep_copy(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    unicode_t *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    int32_t byte = sizeof(unicode_type_t);
    self->capacity = other->capacity;
    self->buffer = calloc(self->capacity + 1, byte);
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
    // self->mb = cstr_edup(other->mb);

    return self;
}

unicode_t *
uni_app_other(unicode_t *self, const unicode_t *_other) {
    if (!self || !_other) {
        return NULL;
    }

    unicode_t *other = uni_deep_copy(_other);
    unicode_t *ret = NULL;

    if (self == other) {
        unicode_type_t *buf = u_strdup(self->buffer);
        if (!buf) {
            uni_del(other);
            return ret;
        }
        ret = uni_app(self, buf);
        free(buf);
    } else {
        ret = uni_app(self, other->buffer);
    }

    uni_del(other);
    return ret;
}

unicode_t *
uni_app_fmt(unicode_t *self, char *buf, int32_t nbuf, const char *fmt, ...) {
    if (!self || !buf || !fmt || nbuf == 0) {
        return NULL;
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, nbuf, fmt, args);
    va_end(args);

    unicode_t *tail = uni_new();
    uni_set_mb(tail, buf);

    for (const unicode_type_t *p = uni_getc(tail); *p; ++p) {
        uni_pushb(self, *p);
    }

    uni_del(tail);
    return self;
}

char *
uni_to_mb(const unicode_t *self) {
    if (!self) {
        return NULL;
    }

    mbstate_t mbstate = {0};
    string_t *buf = str_new();

    for (int32_t i = 0; i < self->length; ++i) {
        char mb[MB_LEN_MAX + 1];
#if defined(UNI_CHAR32)
        size_t result = c32rtomb(mb, self->buffer[i], &mbstate);
#elif defined(UNI_CHAR16)
        size_t result = c16rtomb(mb, self->buffer[i], &mbstate);
#endif
        if (result == -1) {
            return NULL;
        }

        mb[result] = '\0';

        for (const char *p = mb; *p; ++p) {
            str_pushb(buf, *p);
        }
    }

    return str_esc_del(buf);
}

unicode_t *
uni_set_mb(unicode_t *self, const char *mb) {
    if (!self || !mb) {
        return NULL;
    }

    int32_t len = strlen(mb);
    mbstate_t mbstate = {0};
    int mbi = 0;

    uni_clear(self);

    for (; mbi < len;) {
        char32_t c32;
        mbstate = (mbstate_t) {0};
        errno = 0;
        const int result = mbrtoc32(&c32, &mb[mbi], MB_CUR_MAX, &mbstate);
        if (result > 0) {
            mbi += result;
        } else if (result == 0) {
            // reached null terminator
            break;
        } else if (result == -1) {
            // invalid bytes
            fprintf(stderr, "uni_set_mb: invalid characters\n");
            perror("mbrtoc32");
            return NULL;
        } else if (result == -2) {
            fprintf(stderr, "uni_set_mb: incomplete input characters\n");
            return NULL;
        } else if (result == -3) {
            // char32_t の文字を構成する残りの部分を得た。
            // マルチバイト文字側のバイトは消費していない
            fprintf(stderr, "uni_set_mb: got -3\n");
        }

        if (!uni_pushb(self, c32)) {
            return NULL;
        }
    }

    return self;
}

unicode_t *
uni_rstrip(unicode_t *self, const unicode_type_t *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (int32_t i = self->length - 1; i > 0; --i) {
        bool found = false;
        for (const unicode_type_t *p = rems; *p; ++p) {
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

unicode_t *
uni_lstrip(unicode_t *self, const unicode_type_t *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (; self->length; ) {
        bool found = false;
        for (const unicode_type_t *p = rems; *p; ++p) {
            if (*p == self->buffer[0]) {
                uni_popf(self);
                found = true;
            }
        }
        if (!found) {
            break;
        }
    }

    return self;
}

unicode_t *
uni_strip(unicode_t *self, const unicode_type_t *rems) {
    if (!self || !rems) {
        return NULL;
    }

    if (!uni_rstrip(self, rems)) {
        return NULL;
    }

    if (!uni_lstrip(self, rems)) {
        return NULL;
    }

    return self;
}

const char *
uni_getc_mb(unicode_t *self) {
    char *mb = uni_to_mb(self);
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
conv_to_mb(unicode_t *self, char *mb, unicode_type_t ch) {
    if (!self || !mb) {
        return NULL;
    }

    size_t result;
    mbstate_t mbstate = {0};

#if defined(UNI_CHAR32)
    result = c32rtomb(mb, ch, &mbstate);
#elif defined(UNI_CHAR16)
    result = c16rtomb(mb, ch, &mbstate);
#endif

    if (result == -1) {
        return NULL;
    }

    mb[result] = '\0';
    return mb;
}

unicode_t *
uni_lower(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    unicode_t *self = uni_deep_copy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        unicode_type_t ch = self->buffer[i];
        if (u_isupper(ch)) {
            self->buffer[i] = u_tolower(ch);
        }
    }

    return self;
}

unicode_t *
uni_upper(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    unicode_t *self = uni_deep_copy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        unicode_type_t ch = self->buffer[i];
        if (u_islower(ch)) {
            self->buffer[i] = u_toupper(ch);
        }
    }

    return self;
}

unicode_t *
uni_capitalize(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    unicode_t *self = uni_deep_copy(other);
    if (self->length) {
        unicode_type_t ch = self->buffer[0];
        if (u_islower(ch)) {
            self->buffer[0] = u_toupper(ch);
        }
    }

    return self;
}

unicode_t *
uni_snake(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    const unicode_type_t *p = uni_getc(other);
    unicode_t *self = uni_new();

    for (; *p; ++p) {
        switch (m) {
        case 0: // first
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                m = 10;
            } else {
                uni_pushb(self, u_tolower(*p));
                m = 20;
            }
            break;
        case 10: // found '-' or '_'
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                // pass
            } else {
                uni_pushb(self, u_tolower(*p));
                m = 20;
            }
            break;
        case 20: // found normal character
            if (u_isupper(*p)) {
                uni_pushb(self, UNI_CHAR('_'));
                uni_pushb(self, u_tolower(*p));
            } else if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                uni_pushb(self, UNI_CHAR('_'));
                m = 10;
            } else {
                uni_pushb(self, *p);
            }
            break;
        }
    }

    return self;
}

unicode_t *
uni_camel(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    unicode_t *self = uni_new();
    const unicode_type_t *p = uni_getc(other);

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                m = 10;
            } else {
                uni_pushb(self, u_tolower(*p));
                m = 20;
            }
            break;
        case 10:  // found '-' or '_' first
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                // pass
            } else {
                uni_pushb(self, u_tolower(*p));
                m = 20;
            }
            break;
        case 15:  // found '-' or '_' second
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                // pass
            } else {
                uni_pushb(self, u_toupper(*p));
                m = 30;
            }
            break;
        case 20:  // readed lower character
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                m = 15;
            } else if (u_isupper(*p)) {
                uni_pushb(self, *p);
                m = 30;
            } else {
                uni_pushb(self, *p);
            }
            break;
        case 30:  // readed upper character
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                m = 15;
            } else if (u_isupper(*p)) {
                uni_pushb(self, *p);
            } else {
                uni_pushb(self, *p);
                m = 20;
            }
            break;
        }
    }

    return self;
}

unicode_t *
uni_hacker(const unicode_t *other) {
    if (!other) {
        return NULL;
    }

    unicode_t *self = uni_new();
    const unicode_type_t *p = uni_getc(other);
    int m = 0;

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == UNI_CHAR('-') || *p == UNI_CHAR('_')) {
                m = 100;
            } else if (u_isupper(*p)) {
                uni_pushb(self, u_tolower(*p));
            } else if (u_islower(*p)) {
                uni_pushb(self, *p);
            } else if (u_isdigit(*p)) {
                uni_pushb(self, *p);
            } else {
                uni_pushb(self, *p);
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

unicode_t *
uni_mul(const unicode_t *self, int32_t n) {
    if (!self) {
        return NULL;
    }
    
    unicode_t *buf = uni_new();

    for (int32_t i = 0; i < n; ++i) {
        uni_app(buf, self->buffer);
    }

    return buf;
}

#undef NIL
