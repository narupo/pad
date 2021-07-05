/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <pad/lib/error.h>

typedef struct {
    // t ... token ('abc' | '/path/to/dir')
    // d ... dot ('.')
    // s ... space (' ')
    char type;
    string_t *token;
} err_PadTok;

static err_PadTok *
gen_token(char type, string_t *move_token) {
    err_PadTok *tok = mem_calloc(1, sizeof(*tok));
    if (!tok) {
        return NULL;
    }

    tok->type = type;
    tok->token = mem_move(move_token);

    return tok;
}

static char
infer_type(const string_t *tok) {
    if (!str_len(tok)) {
        return 0;
    }

    const char *s = str_getc(tok);
    char last = s[strlen(s) - 1];
    switch (last) {
    default: return 't'; break;
    case '.': return 'd'; break;
    case ' ': return 's'; break;
    }
}

static err_PadTok **
tokenize(const char *src) {
    int32_t capa = 4;
    int32_t cursize = 0;
    err_PadTok **tokens = mem_calloc(capa + 1, sizeof(err_PadTok *));
    if (!tokens) {
        return NULL;
    }
    string_t *buf = str_new();
    char bef = 0;

#define push(t) \
    if (cursize >= capa) { \
        int32_t nbyte = sizeof(err_PadTok); \
        capa *= 2; \
        err_PadTok **tmp = mem_realloc(tokens, capa * nbyte + nbyte); \
        if (!tmp) { \
            free(tokens); \
            return NULL; \
        } \
        tokens = tmp; \
    } \
    tokens[cursize++] = t; \
    tokens[cursize] = NULL; \

#define store \
    if (str_len(buf)) { \
        char type = infer_type(buf); \
        err_PadTok *tok = gen_token(type, mem_move(buf)); \
        if (!tok) { \
            return NULL; \
        } \
        push(tok); \
        buf = str_new(); \
    } \

    for (const char *p = src; *p; ++p) {
        if (*p == ' ') {
            store;
            if (bef != *p) {
                str_pushb(buf, *p);
                store;
            }
        } else if (*p == '.') {
            store;
            for (; *p == '.'; ++p) {
                str_pushb(buf, *p);
            }
            --p;
            store;
        } else if (*p == '"') {
            store;
            str_pushb(buf, *p++);
            for (; *p; ++p) {
                if (*p == '\\') {
                    str_pushb(buf, *p++);
                    str_pushb(buf, *p);
                } else if (*p == '"') {
                    str_pushb(buf, *p);
                    break;
                } else {
                    str_pushb(buf, *p);
                }
            }            
        } else {
            str_pushb(buf, *p);
        }

        bef = *p;
    }

    store;

    str_del(buf);
    return tokens;
}

static bool
look_fname_ext(err_PadTok **p) {
    for (; *p; ++p) {
        err_PadTok *tok = *p;
        err_PadTok *second = *(p + 1);
        char next = 0;
        if (second) {
            next = second->type;
        }
        if (tok->type == 's') {
            return false;
        } else if (tok->type == 'd' && (next != 's' && next != 0)) {
            return true;
        }
    }

    return false;
}

void
err_fix_text(char *dst, uint32_t dstsz, const char *src) {
    const char *deb = getenv("ERROR_DEBUG");
    bool debug = deb && deb[0] == '1';
    int m = 0;

    err_PadTok **tokens = tokenize(src);
    if (!tokens) {
        return;
    }

    for (err_PadTok **p = tokens; *p; ++p) {
        err_PadTok *tok = *p;
        if (debug) {
            printf("m[%d] type[%c] token[%s]\n", m, tok->type, str_getc(tok->token));
        }

        switch (m) {
        case 0:  // ended of dot
            if (tok->type == 't') {
                if (!look_fname_ext(p)) {
                    string_t *copied = str_capitalize(tok->token);
                    cstr_app(dst, dstsz, str_getc(copied));
                    str_del(copied);
                } else {
                    cstr_app(dst, dstsz, str_getc(tok->token));
                }
                m = 10;
            } else if (tok->type == 's') {
                // pass
            } else if (tok->type == 'd') {
                // pass
            }
            break;
        case 10:  // found token
            if (tok->type == 't') {
                cstr_app(dst, dstsz, str_getc(tok->token));
            } else if (tok->type == 's') {
                cstr_app(dst, dstsz, str_getc(tok->token));
            } else if (tok->type == 'd') {
                cstr_app(dst, dstsz, str_getc(tok->token));
                m = 20;
            }
            break;
        case 20:  // found token -> dot
            if (tok->type == 't') {
                cstr_app(dst, dstsz, str_getc(tok->token));
            } else if (tok->type == 's') {
                cstr_app(dst, dstsz, str_getc(tok->token));
                m = 0;
            } else if (tok->type == 'd') {
                cstr_app(dst, dstsz, str_getc(tok->token));
            }
            break;
        }
    }

    int32_t dstlen = strlen(dst);
    if (dst[dstlen - 1] == ' ') {
        dst[dstlen - 1] = '\0';
        dstlen--;
    }
    if (dst[dstlen - 1] != '.') {
        cstr_app(dst, dstsz, ".");
    }

    for (err_PadTok **tok = tokens; *tok; ++tok) {
        free(*tok);
    }
    free(tokens);
}

static void
errorap_unsafe(const char *title, va_list ap, const char *fmt) {
	fflush(stdout);

	uint32_t fmtlen = strlen(fmt);

	if (title != NULL && strlen(title)) {
		fprintf(stderr, "%c%s: ", toupper(title[0]), title+1);
	}

	if (fmtlen) {
        char tmp[1024 * 5] = {0},
             msg[1024 * 5] = {0};
		vsnprintf(tmp, sizeof tmp, fmt, ap);
        err_fix_text(msg, sizeof msg, tmp);
		fprintf(stderr, "%s", msg);
        if (strlen(msg) && msg[strlen(msg)-1] != '.') {
            fprintf(stderr, ".");
        }
	}

	if (errno != 0) {
		fprintf(stderr, " %s. ", strerror(errno));
	}

	fprintf(stderr, "\n");
	fflush(stderr);
}

static const char *
fmttoupper_unsafe(char *dst, uint32_t dstsz, const char *fmt) {
	if (isalpha(fmt[0])) {
		snprintf(dst, dstsz, "%c%s", toupper(fmt[0]), fmt+1);
		return dst;
	}

	return fmt;
}

bool
_log_unsafe(const char *file, long line, const char *func, const char *type, const char *msg) {
	// Check arguments
	type = (type ? type : "type");
	msg = (msg ? msg : "");

	FILE *fout = stderr;
	uint32_t msglen = strlen(msg);

	// Datetime
	time_t tim = time(NULL);
	struct tm tm = *localtime(&tim);
	fprintf(fout, "%d-%d-%d %d:%d:%d",
		tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	// Messages
	fprintf(fout, ": %c%s: %s: %ld: %s", toupper(type[0]), type+1, file, line, func);

	if (errno != 0) {
		fprintf(fout, ": %s", strerror(errno));
	}

	if (msglen) {
		fprintf(fout, ": %c%s", toupper(msg[0]), msg+1);
		if (msg[msglen-1] != '.') {
			fprintf(fout, ".");
		}
	} else {
		fprintf(fout, ".");
	}

	fprintf(fout, "\n");
	fflush(fout);
	return true;
}

void
_err_die(
	const char *fname,
	int32_t line,
	const char *funcname,
	const char *fmt,
	...
) {
	char tmp[1024];
	fmt = fmttoupper_unsafe(tmp, sizeof tmp, fmt);

	char head[1024];
	snprintf(head, sizeof head, "die: %s: %d: %s:", fname, line, funcname);

	va_list ap;
	va_start(ap, fmt);
	errorap_unsafe(head, ap, fmt);
	va_end(ap);

	exit(EXIT_FAILURE);
}

void
err_error(const char *fmt, ...) {
	char tmp[1024];
	fmt = fmttoupper_unsafe(tmp, sizeof tmp, fmt);

	va_list ap;
	va_start(ap, fmt);
	errorap_unsafe("error", ap, fmt);
	va_end(ap);
}

void
err_warn(const char *fmt, ...) {
	char tmp[1024];
	fmt = fmttoupper_unsafe(tmp, sizeof tmp, fmt);

	va_list ap;
	va_start(ap, fmt);
	errorap_unsafe("warn", ap, fmt);
	va_end(ap);
}

void
err_debug(const char *fmt, ...) {
	const char *isdeb = getenv("CAP_DEBUG");
	if (!isdeb || (isdeb && *isdeb == '0')) {
		return;
	}

	char tmp[1024];
	fmt = fmttoupper_unsafe(tmp, sizeof tmp, fmt);

	va_list ap;
	va_start(ap, fmt);
	errorap_unsafe("debug", ap, fmt);
	va_end(ap);
}
