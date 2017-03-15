/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "error.h"

bool
_cap_log(const char *file, long line, const char *func, const char *type, const char *msg) {
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

static void
errorap(va_list ap, const char *fmt) {
	fflush(stdout);

	size_t fmtlen = strlen(fmt);
	const char *procname = getenv("CAP_PROCNAME");
	if (!procname) {
		procname = "cap";
	}
	fprintf(stderr, "%s: ", procname);

	if (errno != 0) {
		fprintf(stderr, "%s. ", strerror(errno));
	}
	
	if (fmtlen) {
		vfprintf(stderr, fmt, ap);
	}

	if (fmtlen && fmt[fmtlen-1] != '.') {
		fprintf(stderr, ".");
	}

	fprintf(stderr, "\n");
	fflush(stderr);
}

static const char *
fmttoupper(char *dst, size_t dstsz, const char *fmt) {
	if (isalpha(fmt[0])) {
		snprintf(dst, dstsz, "%c%s", toupper(fmt[0]), fmt+1);
		return dst;
	}

	return fmt;
}

void
cap_error(const char *fmt, ...) {
	char tmp[1024];
	fmt = fmttoupper(tmp, sizeof tmp, fmt);

	va_list ap;
	va_start(ap, fmt);
	errorap(ap, fmt);
	va_end(ap);
}

void
cap_die(const char *fmt, ...) {
	char tmp[1024];
	fmt = fmttoupper(tmp, sizeof tmp, fmt);

	va_list ap;
	va_start(ap, fmt);
	errorap(ap, fmt);
	va_end(ap);

	exit(EXIT_FAILURE);
}
