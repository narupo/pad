#include "util.h"

void _Noreturn
die(char const* fmt, ...) {
	size_t fmtlen = strlen(fmt);
	va_list args;
	va_start(args, fmt);

	term_flush();

	vfprintf(stderr, fmt, args);

	if (fmtlen && fmt[fmtlen-1] != '.') {
		term_eprintf(". ");
	}
	if (errno != 0) {
		term_eprintf("%s.", strerror(errno));
	}

	term_eprintf("\n");

	va_end(args);

	exit(EXIT_FAILURE);
}

void
warn(char const* fmt, ...) {
	size_t fmtlen = strlen(fmt);
	va_list args;
	va_start(args, fmt);

	fflush(stdout);

	vfprintf(stderr, fmt, args);

	if (fmtlen && fmt[fmtlen-1] != '.') {
		term_eprintf(". ");
	}
	if (errno != 0) {
		term_eprintf("%s.", strerror(errno));
	}

	term_eprintf("\n");

	va_end(args);
}

char*
strdup(char const* src) {
	size_t len = strlen(src);
	char* dst = (char*) malloc(sizeof(char) * len + 1);  // +1 for final '\0'
	if (!dst) {
		return NULL;
	}

	memmove(dst, src, len);
	dst[len] = '\0';

	return dst;
}

char*
strappend(char* dst, size_t dstsize, char const* src) {
	if (!src) {
		return dst;
	}

	size_t dstcur = strlen(dst);  // Weak point

	for (size_t i = 0; dstcur < dstsize-1 && src[i]; ++dstcur, ++i) {
		dst[dstcur] = src[i];
	}
	dst[dstcur] = '\0';

	return dst;
}

const char*
strskip(char const* src, char const* skips) {
	char const* p = src;
	for (; *p; ++p) {
		if (!strchr(skips, *p)) {
			break;
		}
	}
	return p;
}

int
strcmphead(char const* src, char const* target) {
	return strncmp(src, target, strlen(target));
}

char*
strrem(char* dst, size_t dstsize, char const* src, int rem) {
	int i, j;

	for (i = 0, j = 0; src[i]; ++i) {
		if (src[i] != rem) {
			dst[j++] = src[i];
		}
	}

	dst[j] = '\0';

	return dst;
}

char*
strrstrip(char* dst, size_t dstsize, char const* src, int rem) {
	size_t srclen = strlen(src);

	if (srclen >= dstsize) {
		die("strrstrip: buffer overflow");
	}

	int i;
	for (i = 0; i < srclen; ++i) {
		int c = src[srclen-i-1];
		if (c != rem) {
			break;
		}
		dst[srclen-i-1] = c;
	}

	dst[i] = '\0';

	return dst;
}

char*
strrset(char* str, size_t strsize, int rem) {
	for (int i = strlen(str)-1; i >= 0; --i) {
		if (str[i] != rem) {
			break;
		}
		str[i] = '\0';
	}
	return str;
}

void
free_argv(int argc, char** argv) {
	if (argv) {
		for (int i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
	}
}

#if defined(TEST_UTIL)
int
main(int argc, char* argv[]) {
	char src[] = "\t \t \t456";
	char const* skips = " \t";
	char* p;

	p = strskip(src, skips);
	printf("p[%s]\n", p);

	p = strskip(p, skips);
	printf("p[%s]\n", p);

	p = strskip(p, skips);
	printf("p[%s]\n", p);

    return 0;
}
#endif
