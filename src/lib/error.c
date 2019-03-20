/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "error.h"

static char *
capitalize_text(char *dst, size_t dstsz, const char *text) {
    int m = 0;
    char *dp = dst;
    char *dpend = dst + (dstsz-1);
    const char *p = text;

    for (; *p && dp < dpend; ) {
        char c = *p++;
        switch (m) {
        case 0: // first
            if (isspace(c)) {
                *dp++ = c;
            } else {
                if (isalpha(c)) {
                    *dp++ = toupper(c);
                } else {
                    *dp++ = c;
                }
                m = 10;
            }
            break;
        case 10: // found character
            if (c == '.') {
                *dp++ = c;
                m = 20;
            } else {
                *dp++ = c;
            }
            break;
        case 20: // found '.'
            if (isalpha(c)) {
                *dp++ = toupper(c);
                m = 10;
            } else {
                *dp++ = c;
            }
            break;
        }
    }

    *dp = '\0';
    return dst;
}

static void
errorap_unsafe(const char *title, va_list ap, const char *fmt) {
	fflush(stdout);

	size_t fmtlen = strlen(fmt);

	if (title != NULL && strlen(title)) {
		fprintf(stderr, "%c%s: ", toupper(title[0]), title+1);
	}

	if (fmtlen) {
		char tmp[1024*5] = {0};
		vsnprintf(tmp, sizeof tmp, fmt, ap);
		capitalize_text(tmp, sizeof tmp, tmp);
		fprintf(stderr, "%s", tmp);
	}

	if (fmtlen && fmt[fmtlen-1] != '.') {
		fprintf(stderr, ".");
	}

	if (errno != 0) {
		fprintf(stderr, " %s. ", strerror(errno));
	}
	
	fprintf(stderr, "\n");
	fflush(stderr);
}

static const char *
fmttoupper_unsafe(char *dst, size_t dstsz, const char *fmt) {
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
	size_t msglen = strlen(msg);

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
err_die(const char *fmt, ...) {
	char tmp[1024];
	fmt = fmttoupper_unsafe(tmp, sizeof tmp, fmt);

	va_list ap;
	va_start(ap, fmt);
	errorap_unsafe(NULL, ap, fmt);
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
