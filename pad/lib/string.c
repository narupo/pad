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

struct string {
    int length;
    int capacity;
    string_type_t *buffer;
};

/*************
* str macros *
*************/

#define NCHAR (sizeof(string_type_t))
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
str_del(string_t *self) {
    if (!self) {
        return;
    }

    free(self->buffer);
    free(self);
}

string_type_t *
str_esc_del(string_t *self) {
    if (!self) {
        return NULL;
    }

    string_type_t *buf = self->buffer;
    free(self);
    return buf;
}

string_t *
str_new(void) {
    string_t *self = calloc(1, sizeof(string_t));
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

string_t *
str_new_cstr(const string_type_t *str) {
    if (!str) {
        return NULL;
    }

    string_t *self = str_new();
    str_set(self, str);
    return self;
}

string_t *
str_deep_copy(const string_t *other) {
    if (!other) {
        return NULL;
    }

    string_t *self = calloc(1, sizeof(string_t));
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

string_t *
str_shallow_copy(const string_t *other) {
    return str_deep_copy(other);
}

/*************
* str getter *
*************/

int32_t
str_len(const string_t *self) {
    if (!self) {
        return -1;
    }
    return self->length;
}

int32_t
str_capa(const string_t *self) {
    if (!self) {
        return -1;
    }
    return self->capacity;
}

const string_type_t *
str_getc(const string_t *self) {
    if (!self) {
        return NULL;
    }
    return self->buffer;
}

int32_t
str_empty(const string_t *self) {
    if (!self) {
        return 0;
    }
    return self->length == 0;
}

/*************
* str setter *
*************/

void
str_clear(string_t *self) {
    if (!self) {
        return;
    }

    self->length = 0;
    self->buffer[self->length] = NIL;
}

string_t *
str_set(string_t *self, const string_type_t *src) {
    if (!self || !src) {
        return NULL;
    }

    int srclen = strlen(src);
    if (srclen >= self->length) {
        if (!str_resize(self, srclen)) {
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

string_t *
str_resize(string_t *self, int32_t newcapa) {
    if (!self) {
        return NULL;
    }

    if (newcapa < 0) {
        newcapa = 0;
    }

    string_type_t *tmp = realloc(self->buffer, newcapa*NCHAR + NCHAR); // +NCHAR for final nil
    if (!tmp) {
        str_del(self);
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

string_t *
str_pushb(string_t *self, string_type_t ch) {
    if (!self || ch == NIL) {
        return NULL;
    }

    if (self->length >= self->capacity-1) {
        if (!str_resize(self, self->length*2)) {
            return NULL;
        }
    }

    self->buffer[self->length++] = ch;
    self->buffer[self->length] = NIL;

    return self;
}

string_type_t
str_popb(string_t *self) {
    if (!self) {
        return NIL;
    }

    if (self->length > 0) {
        string_type_t ret = self->buffer[--self->length];
        self->buffer[self->length] = NIL;
        return ret;
    }

    return NIL;
}

string_t *
str_pushf(string_t *self, string_type_t ch) {
    if (!self || ch == NIL) {
        return NULL;
    }

    if (self->length >= self->capacity-1) {
        if (!str_resize(self, self->length*2)) {
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

string_type_t
str_popf(string_t *self) {
    if (!self || self->length == 0) {
        return NIL;
    }

    string_type_t ret = self->buffer[0];

    for (int32_t i = 0; i < self->length-1; ++i) {
        self->buffer[i] = self->buffer[i+1];
    }

    --self->length;
    self->buffer[self->length] = NIL;

    return ret;
}

string_t *
str_app(string_t *self, const string_type_t *src) {
    if (!self || !src) {
        return NULL;
    }

    int32_t srclen = strlen(src);

    if (self->length + srclen >= self->capacity-1) {
        if (!str_resize(self, (self->length + srclen) * 2)) {
            return NULL;
        }
    }

    for (const string_type_t *sp = src; *sp; ++sp) {
        self->buffer[self->length++] = *sp;
    }
    self->buffer[self->length] = NIL;

    return self;
}

string_t *
str_app_stream(string_t *self, FILE *fin) {
    if (!self || !fin) {
        return NULL;
    }

    for (int32_t ch; (ch = fgetc(fin)) != EOF; ) {
        if (!str_pushb(self, ch)) {
            return NULL;
        }
    }

    return self;
}

string_t *
str_app_other(string_t *self, const string_t *_other) {
    if (!self || !_other) {
        return NULL;
    }

    string_t *other = str_deep_copy(_other);
    string_t *ret = NULL;

    if (self == other) {
        string_type_t *buf = cstr_edup(self->buffer);
        if (!buf) {
            str_del(other);
            return ret;
        }
        ret = str_app(self, buf);
        free(buf);
    } else {
        ret = str_app(self, other->buffer);
    }

    str_del(other);
    return ret;
}

string_t *
str_app_fmt(string_t *self, string_type_t *buf, int32_t nbuf, const string_type_t *fmt, ...) {
    if (!self || !buf || !fmt || nbuf == 0) {
        return NULL;
    }

    va_list args;
    va_start(args, fmt);
    int32_t buflen = vsnprintf(buf, nbuf, fmt, args);
    va_end(args);

    for (int32_t i = 0; i < buflen; ++i) {
        if (!str_pushb(self, buf[i])) {
            return NULL;
        }
    }

    return self;
}

static string_t *
_str_rstrip(string_t *self, const string_type_t *rems) {
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

string_t *
str_rstrip(const string_t *other, const string_type_t *rems) {
    if (!other || !rems) {
        return NULL;
    }

    string_t *dst = str_deep_copy(other);
    if (!_str_rstrip(dst, rems)) {
        str_del(dst);
        return NULL;
    }

    return dst;
}

static string_t *
_str_lstrip(string_t *self, const string_type_t *rems) {
    if (!self || !rems) {
        return NULL;
    }

    for (; self->length; ) {
        if (strchr(rems, self->buffer[0])) {
            str_popf(self);
        } else {
            break;
        }
    }

    return self;
}

string_t *
str_lstrip(const string_t *other, const string_type_t *rems) {
    if (!other || !rems) {
        return NULL;
    }

    string_t *dst = str_deep_copy(other);
    if (!_str_lstrip(dst, rems)) {
        str_del(dst);
        return NULL;
    }

    return dst;
}

string_t *
str_strip(const string_t *other, const string_type_t *rems) {
    if (!other || !rems) {
        return NULL;
    }

    string_t *dst = str_deep_copy(other);

    if (!_str_rstrip(dst, rems)) {
        str_del(dst);
        return NULL;
    }

    if (!_str_lstrip(dst, rems)) {
        str_del(dst);
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
static const string_type_t *
bmfind(
    const string_type_t *restrict tex,
    int32_t texlen,
    const string_type_t *restrict pat,
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

const string_type_t *
str_findc(const string_t *self, const string_type_t *target) {
    if (!self || !target) {
        return NULL;
    }

    return bmfind(self->buffer, self->length, target, strlen(target));
}

string_t *
str_lower(const string_t *other) {
    if (!other) {
        return NULL;
    }

    string_t *self = str_deep_copy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        string_type_t ch = self->buffer[i];
        if (isupper(ch)) {
            self->buffer[i] = tolower(ch);
        }
    }

    return self;
}

string_t *
str_upper(const string_t *other) {
    if (!other) {
        return NULL;
    }

    string_t *self = str_deep_copy(other);
    for (int32_t i = 0; i < self->length; ++i) {
        string_type_t ch = self->buffer[i];
        if (islower(ch)) {
            self->buffer[i] = toupper(ch);
        }
    }

    return self;
}

string_t *
str_capitalize(const string_t *other) {
    if (!other) {
        return NULL;
    }

    string_t *self = str_deep_copy(other);
    if (self->length) {
        string_type_t ch = self->buffer[0];
        if (islower(ch)) {
            self->buffer[0] = toupper(ch);
        }
    }

    return self;
}

string_t *
str_snake(const string_t *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    const string_type_t *p = str_getc(other);
    string_t *self = str_new();

    for (; *p; ++p) {
        switch (m) {
        case 0: // first
            if (*p == '-' || *p == '_') {
                m = 10;
            } else {
                str_pushb(self, tolower(*p));
                m = 20;
            }
            break;
        case 10: // found '-' or '_'
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                str_pushb(self, tolower(*p));
                m = 20;
            }
            break;
        case 20: // found normal character
            if (isupper(*p)) {
                str_pushb(self, '_');
                str_pushb(self, tolower(*p));
            } else if (*p == '-' || *p == '_') {
                str_pushb(self, '_');
                m = 10;
            } else {
                str_pushb(self, *p);
            }
            break;
        }
    }

    return self;
}

string_t *
str_camel(const string_t *other) {
    if (!other) {
        return NULL;
    }

    int m = 0;
    string_t *self = str_new();
    const string_type_t *p = str_getc(other);

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == '-' || *p == '_') {
                m = 10;
            } else {
                str_pushb(self, tolower(*p));
                m = 20;
            }
            break;
        case 10:  // found '-' or '_' first
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                str_pushb(self, tolower(*p));
                m = 20;
            }
            break;
        case 15:  // found '-' or '_' second
            if (*p == '-' || *p == '_') {
                // pass
            } else {
                str_pushb(self, toupper(*p));
                m = 30;
            }
            break;
        case 20:  // readed lower character
            if (*p == '-' || *p == '_') {
                m = 15;
            } else if (isupper(*p)) {
                str_pushb(self, *p);
                m = 30;
            } else {
                str_pushb(self, *p);
            }
            break;
        case 30:  // readed upper character
            if (*p == '-' || *p == '_') {
                m = 15;
            } else if (isupper(*p)) {
                str_pushb(self, *p);
            } else {
                str_pushb(self, *p);
                m = 20;
            }
            break;
        }
    }

    return self;
}

string_t *
str_hacker(const string_t *other) {
    if (!other) {
        return NULL;
    }

    string_t *self = str_new();
    const string_type_t *p = str_getc(other);
    int m = 0;

    for (; *p; ++p) {
        switch (m) {
        case 0:  // first
            if (*p == '-' || *p == '_') {
                m = 100;
            } else if (isupper(*p)) {
                str_pushb(self, tolower(*p));
            } else if (islower(*p)) {
                str_pushb(self, *p);
            } else if (isdigit(*p)) {
                str_pushb(self, *p);
            } else {
                str_pushb(self, *p);
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

string_t *
str_mul(const string_t *self, int32_t n) {
    if (!self) {
        return NULL;
    }
    
    string_t *buf = str_new();

    for (int32_t i = 0; i < n; ++i) {
        str_app(buf, self->buffer);
    }

    return buf;
}

string_t *
str_indent(const string_t *other, int32_t ch, int32_t n, int32_t tabsize) {
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

    string_t *self = str_new();
    const char *p = str_getc(other);

    str_app(self, value);
    for (; *p; ++p) {
        if (*p == '\r' && *(p + 1) == '\n') {
            ++p;
            str_app(self, "\r\n");
            if (*(p + 1)) {
                str_app(self, value);
            }
        } else if (*p == '\r' || *p == '\n') {
            str_pushb(self, *p);
            if (*(p + 1)) {
                str_app(self, value);
            }
        } else {
            str_pushb(self, *p);
        }
    }

    return self;
}

/**************
* str cleanup *
**************/

#undef NCHAR
#undef NIL
