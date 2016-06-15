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
		fprintf(fout, ": %s", msg);
		if (msg[msglen-1] != '.') {
			fprintf(fout, ".");
		}
	} else {
		fprintf(fout, ".");
	}

	fprintf(fout, "\n");
	return true;
}

void
cap_die(char const *fmt, ...) {
	size_t fmtlen = strlen(fmt);
	va_list args;
	va_start(args, fmt);

	fflush(stdout);

	fprintf(stderr, "Die: ");
	vfprintf(stderr, fmt, args);

	if (fmtlen && fmt[fmtlen-1] != '.') {
		fprintf(stderr, ".");
	}
	if (errno != 0) {
		fprintf(stderr, " %s.", strerror(errno));
	}

	fprintf(stderr, "\n");

	va_end(args);

	fflush(stderr);
	exit(EXIT_FAILURE);
}
