#include "caperr.h"

enum {
	NSTACK = 20,
	NHEADER = 32,
	NMESSAGE = 64,
};

typedef struct {
	int saveerrno;
	int number;
	char header[NHEADER];
	char message[NMESSAGE];
} CapErr;

static struct {
	int stack_top;
	CapErr stack[NSTACK];
} caperrs;

static pthread_mutex_t
caperrs_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool
caperrs_lock(void) {
	if (pthread_mutex_lock(&caperrs_mutex) == 0) {
		return true;
	}

	perror("pthread_mutex_lock");
	return false;
}

static bool
caperrs_unlock(void) {
	if (pthread_mutex_unlock(&caperrs_mutex) == 0) {
		return true;
	}

	perror("pthread_mutex_unlock");
	return false;
}

static void
caperrs_push(int saveerrno, char const* header, int number, char const* fmt, va_list args) {
	if (caperrs_lock()) {
		if (caperrs.stack_top >= NSTACK) {
			term_eputsf("caperr: Stack overflow");
		} else {
			CapErr* s = &caperrs.stack[caperrs.stack_top++];
			s->saveerrno = saveerrno;
			s->number = number;
			snprintf(s->header, sizeof(s->header), "%s", header);
			vsnprintf(s->message, sizeof(s->message), fmt, args);
		}
		caperrs_unlock();
	}
}

static CapErr const*
caperrs_pop(void) {
	if (caperrs_lock()) {
		if (caperrs.stack_top <= 0) {
			caperrs_unlock();
			return NULL;
		}

		caperrs_unlock();
		return &caperrs.stack[--caperrs.stack_top];
	}

	return NULL;
}

int
caperr(char const* header, int number, char const* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	caperrs_push(errno, header, number, fmt, args);
	va_end(args);

	return number;
}

char const*
caperr_to_string(int number) {
	switch (number) {
		case CAPERR_CONSTRUCT: return "Failed to construct"; break;
		case CAPERR_FOPEN: return "Failed to open file"; break;
		case CAPERR_PARSE_OPTIONS: return "Failed to parse options"; break;
		case CAPERR_INVALID_ARGUMENTS: return "Invalid arguments"; break;
	}
	return "Unknown errors";
}

void
caperr_display(void) {
	for (CapErr const* e = caperrs_pop(); e; e = caperrs_pop()) {
		fprintf(stderr, "%s: %s.", e->header, caperr_to_string(e->number));
		size_t msglen = strlen(e->message);

		if (msglen) {
			fputc(' ', stderr);
			
			if (islower(e->message[0])) {
				fputc(toupper(e->message[0]), stderr);
			}

			fprintf(stderr, "%s", e->message + 1);
			
			if (e->message[msglen-1] != '.') {
				fprintf(stderr, ".");
			}
		}
		
		if (e->saveerrno != 0) {
			fprintf(stderr, " %s.", strerror(e->saveerrno));
		}
		
		fprintf(stderr, "\n");
	}

	caperrs_unlock();
}

#if defined(TEST_CAPERR)
int
main(int argc, char* argv[]) {
	caperr(argv[0], CAPERR_FOPEN, "not found file \"%s\"", argv[0]);
	caperr(argv[0], CAPERR_CONSTRUCT, "not found file \"%s\"", argv[0]);
	caperr(argv[0], CAPERR_PARSE_OPTIONS, "not found file \"%s\"", argv[0]);
	caperr_display();
    return 0;
}
#endif
