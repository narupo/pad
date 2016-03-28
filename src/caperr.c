#include "caperr.h"

enum {
	NSTACK = 20,
	NFNAME = 32,
	NFUNCNAME = 32,
	NHEADER = 32,
	NMESSAGE = 128,
};

typedef struct {
	int saveerrno;
	int number; // number of CAPERR
	int lineno; // number of line in file
	char fname[NFNAME]; // file name
	char funcname[NFUNCNAME]; // function name in file
	char header[NHEADER]; // line header message
	char body[NMESSAGE]; // line body message
} CapErr;

static struct {
	int stack_top;
	CapErr stack[NSTACK];
} caperrs;

static pthread_mutex_t caperrs_mutex = PTHREAD_MUTEX_INITIALIZER;

/********************
* caperr prototypes *
********************/

char const*
caperr_to_string_unsafe(int number);

/**************
* caperr util *
**************/

static bool
caperrs_lock(void) {
	if (pthread_mutex_lock(&caperrs_mutex) == 0) {
		return true;
	}
	return false;
}

static bool
caperrs_unlock(void) {
	if (pthread_mutex_unlock(&caperrs_mutex) == 0) {
		return true;
	}
	return false;
}

/***************
* caperr stack *
***************/

/**
 * Push to stack of caperrs
 * This function is multi-thread safe
 *
 * @param[in] saveerrno
 * @param[in] *fname
 * @param[in] *funcname
 * @param[in] lineno
 * @param[in] *header
 * @param[in] number
 * @param[in] *fmt
 * @param[in] args
 */
static void
caperrs_push(
	int saveerrno,
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	va_list args) {

#if !defined(_CAP_DEBUG)
	if (number == CAPERR_DEBUG) {
		return;
	}
#endif

	if (!caperrs_lock()) {
		perror("caperr lock failed");
		return;
	}

	if (caperrs.stack_top >= NSTACK) {
		caperr_display(stderr);
		perror("caperr: Stack overflow");

	} else {
		CapErr* s = &caperrs.stack[caperrs.stack_top++];
		s->saveerrno = saveerrno;
		s->number = number;
		s->lineno = lineno;
		snprintf(s->fname, sizeof(s->fname), "%s", fname);
		snprintf(s->funcname, sizeof(s->funcname), "%s", funcname);
		snprintf(s->header, sizeof(s->header), "%s", header);
		vsnprintf(s->body, sizeof(s->body), fmt, args);
	}

	if (!caperrs_unlock()) {
		perror("caperr unlock failed");
	}
}

static CapErr const*
caperrs_pop(void) {
	if (!caperrs_lock()) {
		perror("caperr lock failed");
		return NULL;
	}

	// Critical section

	CapErr const* err = NULL;

	if (caperrs.stack_top <= 0) {
		goto done;
	}

	err = &caperrs.stack[--caperrs.stack_top];

	// Done of critical section
done:
	if (!caperrs_unlock()) {
		perror("caperr unlock failed");
	}

	return err;
}

int
_caperr(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...) {

	va_list args;

	va_start(args, fmt);
	caperrs_push(errno, fname, funcname, lineno, header, number, fmt, args);
	va_end(args);

	return number;
}

int
_caperr_printf(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...) {

	if (!caperrs_lock()) {
		perror("Failed to lock caperrs");
		return -1;
	}

#ifdef _CAP_DEBUG
	term_eprintf("%s: %s: %d: ", fname, funcname, lineno);
#endif

	char const* what = caperr_to_string_unsafe(number);
	term_eprintf("%s: %s ", header, what);

	size_t fmtlen = strlen(fmt);
	if (fmtlen) {
		va_list args;
		va_start(args, fmt);

		vfprintf(stderr, fmt, args);

		va_end(args);

		if (fmt[fmtlen-1] != '.') {
			term_eprintf(".");
		}
	}

	if (errno != 0) {
		term_eprintf(" %s.", strerror(errno));
	}

	if (!caperrs_unlock()) {
		term_eprintf("Failed to unlock of caperrs");
	}

	return number;
}

char const*
caperr_to_string_unsafe(int number) {
	switch (number) {
		default: return "Unknown errors"; break;
		case CAPERR_DEBUG: return "debug: "; break;
		case CAPERR_ERROR: return ""; break;
		case CAPERR_IS_EXISTS: return "File was exists"; break;
		case CAPERR_NOTFOUND: return "Not found"; break;
		case CAPERR_CONSTRUCT: return "Failed to construct"; break;
		case CAPERR_FOPEN: return "Failed to open file"; break;
		case CAPERR_OPEN: return "Failed to open"; break;
		case CAPERR_OPENDIR: return "Failed to open directory"; break;
		case CAPERR_FCLOSE: return "Failed to close file"; break;
		case CAPERR_SYNTAX: return "Syntax error"; break;
		case CAPERR_PARSE: return "Failed to parse"; break;
		case CAPERR_PARSE_OPTIONS: return "Failed to parse options"; break;
		case CAPERR_INVALID: return "Invalid"; break;
		case CAPERR_INVALID_ARGUMENTS: return "Invalid arguments"; break;
		case CAPERR_MAKE: return "Failed to make"; break;
		case CAPERR_MAKEDIR: return "Failed to make directory"; break;
		case CAPERR_READ: return "Failed to read"; break;
		case CAPERR_READDIR: return "Failed to read directory"; break;
		case CAPERR_WRITE: return "Failed to write"; break;
		case CAPERR_REMOVE: return "Failed to remove"; break;
		case CAPERR_EXECUTE: return "Failed to execute"; break;
		case CAPERR_SOLVE: return "Failed to solve"; break;
	}
}

static void
caperr_display_record_unsafe(FILE* stream, CapErr const* e) {
#ifdef _CAP_DEBUG
	fprintf(stream, "%s: %s: %d: ", e->fname, e->funcname, e->lineno);
#endif

	// Display header and number
	term_acfprintf(stream, TA_BRIGHT, TC_RED, TC_DEFAULT, "%s: ", e->header);
	term_acfprintf(stream, TA_BRIGHT, TC_YELLOW, TC_DEFAULT, "%s", caperr_to_string_unsafe(e->number));

	// Display user's message
	size_t msglen = strlen(e->body);

	if (!msglen) {
		fprintf(stream, ".");
	} else {
		if (e->number != CAPERR_ERROR) {
			fputc(' ', stream);
		}

		// Display
		term_acfprintf(stream, TA_BRIGHT, TC_YELLOW, TC_DEFAULT, "%s", e->body);

		// Fix tail format of string
		if (e->body[msglen-1] != '.') {
			fprintf(stream, ".");
		}
	}

	// Display by errno
	if (e->saveerrno != 0) {
		fprintf(stream, " %s.", strerror(e->saveerrno));
	}

	// Done
	fprintf(stream, "\n");
}

int
caperr_length(void) {
	int len;

	if (!caperrs_lock()) {
		return -1;
	}

	len = caperrs.stack_top;

	if (!caperrs_unlock()) {
		return -2;
	}

	return len;
}

void
caperr_display(FILE* stream) {
	// Stack trace
	for (CapErr const* e = caperrs_pop(); e; e = caperrs_pop()) {
		caperr_display_record_unsafe(stream, e);
	}
}

void
caperr_display_first(FILE* stream) {
	if (!caperrs_lock()) {
		perror("caperr lock failed");
		return;
	}

	if (caperrs.stack_top != 0) {
		caperr_display_record_unsafe(stream, &caperrs.stack[0]);
	}

	if (!caperrs_unlock()) {
		perror("caperr unlock failed");
	}
}

void
caperr_display_last(FILE* stream) {
	if (!caperrs_lock()) {
		perror("caperr lock failed");
		return;
	}

	if (caperrs.stack_top != 0) {
		caperr_display_record_unsafe(stream, &caperrs.stack[caperrs.stack_top-1]);
	}

	if (!caperrs_unlock()) {
		perror("caperr unlock failed");
	}
}

#if defined(TEST_CAPERR)
int
main(int argc, char* argv[]) {
	caperr(argv[0], CAPERR_FOPEN, "not found file \"%s\"", argv[0]);
	caperr_display();
    return 0;
}
#endif
