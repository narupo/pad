#include "caperr.h"

typedef struct {
	int saveerrno;
	int number;
	char const* header;
	char message[64];
} CapErr;

enum { NSTACK = 20 };
static int stack_top;
CapErr stack[NSTACK];

static void
stack_push(int saveerrno, char const* header, int number, char const* fmt, va_list args) {
	if (stack_top >= NSTACK) {
		fprintf(stderr, "caperr: stack overflow\n");
		return;
	} else {
		CapErr* s = &stack[stack_top++];
		s->saveerrno = saveerrno;
		s->number = number;
		s->header = header;
		vsnprintf(s->message, sizeof(s->message), fmt, args);
	}
}

static CapErr const*
stack_pop(void) {
	if (stack_top <= 0) {
		return NULL;
	} else {
		return &stack[--stack_top];
	}
}

int
caperr(char const* header, int number, char const* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	stack_push(errno, header, number, fmt, args);
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
	for (CapErr const* e = stack_pop(); e; e = stack_pop()) {
		fprintf(stderr, "%s: %s.", e->header, caperr_to_string(e->number));
		size_t msglen = strlen(e->message);
		if (msglen) {
			fprintf(stderr, " %s", e->message);
			if (e->message[msglen-1] != '.') {
				fprintf(stderr, ".");
			}
		}
		if (e->saveerrno != 0) {
			fprintf(stderr, " %s.", strerror(e->saveerrno));
		}
		fprintf(stderr, "\n");
	}
}
