/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include <lib/error.h>

static bool
look_fname_ext(const char *p) {
    for (; *p; ++p) {
        char c = *(p + 1);
        if (*p == ' ') {
            return false;
        } else if (*p == '.' && (isalpha(c) || isdigit(c))) {
            return true;
        }
    }

    return false;
}

void
err_fix_text(char *dst, uint32_t dstsz, const char *src) {
    char *dst2 = mem_ecalloc(1, sizeof(char)*dstsz);
    const char *dend = dst2+dstsz-1;
    const char *sp = src;
    char *dp = dst2;
    int m = 0;
    const char *deb = getenv("ERROR_DEBUG");
    bool debug = deb && deb[0] == '1';

    for (; *sp && dp < dend; ++sp) {
        if (debug) {
            printf("m[%d] c[%c]\n", m, *sp);
        }

        if (m == 0) {
            if (isspace(*sp)) {
                // pass
            } else if (*sp == '"') {
                *dp++ = *sp;
                m = 100;
            } else {
                if (isalpha(*sp)) {
                    if (!look_fname_ext(sp)) {
                        *dp++ = toupper(*sp);
                    } else {
                        *dp++ = *sp;
                        continue;
                    }
                } else {
                    *dp++ = *sp;
                }
                m = 10;
            }
        } else if (m == 10) { // found printable character
            if (*sp == '"') {
                *dp++ = *sp;
                m = 100;
            } else if (*sp == '.') {
                *dp++ = *sp;
                m = 150;
            } else {
                *dp++ = *sp;
            }
        } else if (m == 100) { // found string
            if (*sp == '"') {
                *dp++ = *sp;
                m = 10;
            } else {
                *dp++ = *sp;
            }
        } else if (m == 150) { // found .
            if (isspace(*sp)) {
                // pass
            } else if (*sp == '.' && *(sp + 1) == '.') {
                *dp++ = *sp++;
                *dp++ = *sp;
                if (!(*sp != ' ' && *sp != '\0')) {
                    *dp++ = ' ';
                }
            } else if (*sp == '.') {
                // pass
            } else if (*sp == '"') {
                *dp++ = ' '; // add space after dot
                *dp++ = *sp;
                m = 100;
            } else {
                *dp++ = ' '; // add space after dot
                if (isalpha(*sp)) {
                    if (!look_fname_ext(sp)) {
                        *dp++ = toupper(*sp);
                    } else {
                        *dp++ = *sp;
                        continue;
                    }
                } else {
                    *dp++ = *sp;
                }
                m = 10;
            }
        }
    }

    if (*(dp-1) != '.') {
        *dp++ = '.';
    }
    *dp = '\0';

    memmove(dst, dst2, dstsz);
    free(dst2);
}

static void
errorap_unsafe(const char *title, va_list ap, const char *fmt) {
	fflush(stdout);

	uint32_t fmtlen = strlen(fmt);

	if (title != NULL && strlen(title)) {
		fprintf(stderr, "%c%s: ", toupper(title[0]), title+1);
	}

	if (fmtlen) {
        char tmp[1024*5] = {0};
		vsnprintf(tmp, sizeof tmp, fmt, ap);
        err_fix_text(tmp, sizeof tmp, tmp);
		fprintf(stderr, "%s", tmp);
        if (strlen(tmp) && tmp[strlen(tmp)-1] != '.') {
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
