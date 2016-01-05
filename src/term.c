#include "term.h"

void
term_destroy(void) {
}

int
term_init(void) {
	return 0;
}

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
term_putsf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stdout, fmt, args);
	len += fprintf(stdout, "\n");

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

int
term_eputsf(char const* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int len = vfprintf(stderr, fmt, args);
	len += fprintf(stderr, "\n");
	fflush(stderr);

	va_end(args);

	return len;
}

#if defined(TEST_TERM)
int
main(int argc, char* argv[]) {
	term_init();

	term_destroy();
    return 0;
}
#endif
