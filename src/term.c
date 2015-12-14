#include "term.h"

void
term_flush(void) {
	fflush(stdout);
}

void
term_eflush(void) {
	fflush(stderr);
}

int
term_printf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stdout, fmt, args);

	va_end(args);

	return len;
}

int
term_eprintf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stderr, fmt, args);
	fflush(stderr);

	va_end(args);

	return len;
}
