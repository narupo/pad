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
	fprintf(fout, ": %c%s: %s: %ld: In %s function", toupper(type[0]), type+1, file, line, func);

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
cap_die(const char *fmt, ...) {
	char tmp[1024];
	if (isalpha(fmt[0])) {
		snprintf(tmp, sizeof tmp, "%c%s", toupper(fmt[0]), fmt+1);
		fmt = tmp;
	}

	size_t fmtlen = strlen(fmt);
	va_list args;
	va_start(args, fmt);

	fflush(stdout);

	fprintf(stderr, "cap: ");

	if (errno != 0) {
		fprintf(stderr, "%s. ", strerror(errno));
	}
	
	if (fmtlen) {
		vfprintf(stderr, fmt, args);
	}

	if (fmtlen && fmt[fmtlen-1] != '.') {
		fprintf(stderr, ".");
	}

	fprintf(stderr, "\n");

	va_end(args);

	fflush(stderr);
	exit(EXIT_FAILURE);
}
