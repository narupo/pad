#include "args.h"

/*********
* number *
*********/

enum {
	NCAPA = 4,
};

/******
* arg *
******/

struct cap_arg {
	cap_argtype_t type;
	char *value;
};

void
cap_argdel(struct cap_arg *self) {
	if (self) {
		free(self->value);
		free(self);
	}
}

struct cap_arg *
cap_argnew(void) {
	struct cap_arg *self = calloc(1, sizeof(*self));
	return self;
}

struct cap_arg *
cap_argnewother(const struct cap_arg *other) {
	struct cap_arg *self = cap_argnew();
	if (!self) {
		return NULL;
	}

	self->type = other->type;
	self->value = strdup(other->value);
	if (!self->value) {
		cap_argdel(self);
		return NULL;
	}

	return self;
}

cap_argtype_t
cap_argtype(const struct cap_arg *self) {
	return self->type;
}

const char *
cap_argvaluec(const struct cap_arg *self) {
	return self->value;
}

static bool
isnormch(int ch) {
	return isalnum(ch) || ch == '-' || ch == '_';
}

struct cap_arg *
cap_argparse(struct cap_arg *self, const char *src) {
	if (self->value) {
		free(self->value);
	}

	self->value = strdup(src);
	if (!self->value) {
		return NULL;
	}

	// Parse for arg type
	int m = 0;
	cap_argtype_t type = ARG_UNKNOWN;
	const char *p = self->value;
	do {
		switch (m) {
		case 0: // First
			if (*p == '-') {
				m = 10;
			} else {
				type = ARG_ARGUMENT;
				goto done;
			}
		break;
		case 10: // -
			if (*p == '-') {
				m = 20;
			} else if (isnormch(*p)) {
				type = ARG_SHORTOPT;
				m = 15;
			} else {
				type = ARG_ARGUMENT;
				goto done;
			}
		break;
		case 15: // -?
			if (isnormch(*p)) {
				type = ARG_SHORTOPTS;
				m = 17;
			} else if (*p == '=') {
				type = ARG_SHORTOPTASS;
			} else {
				goto done;
			}
		break;
		case 17: // -??
			if (isnormch(*p)) {
				;
			} else if (*p == '=') {
				type = ARG_SHORTOPTSASS;
				goto done;
			} else {
				goto done;
			}
		break;
		case 20: // --
			if (isnormch(*p)) {
				type = ARG_LONGOPT;
				m = 25;
			} else {
				type = ARG_ARGUMENT;
				goto done;
			}
		break;
		case 25: // --?
 			if (isnormch(*p)) {
 				;
 			} else if (*p == '=') {
 				type = ARG_LONGOPTASS;
 				goto done;
 			} else {
 				goto done;
 			}
		break;
		}
	} while (*p++);

done:
	self->type = type;
	return self;
}

void
cap_argshow(const struct cap_arg *self, FILE *fout) {
	fprintf(fout, "<cap_arg type=\"%d\" value=\"%s\">\n", self->type, self->value);
}

struct cap_arg *
cap_argwrapvalue(const struct cap_arg *self, int wrpch) {
	if (!self->value) {
		return NULL;
	}

	const char *src = self->value;
	int srclen = strlen(src);
	int dstcapa = srclen+2+1;
	char *dst = calloc(dstcapa, sizeof(self->value[0])); // +1 for final nul
	if (!dst) {
		return NULL;
	}

	switch (self->type) {
	default: {
		snprintf(dst, dstcapa, "%s", src);
		goto done;
	} break;
	case ARG_ARGUMENT: {
		snprintf(dst, dstcapa, "%c%s%c", wrpch, src, wrpch);
	} break;
	case ARG_SHORTOPTASS:
	case ARG_SHORTOPTSASS:
	case ARG_LONGOPTASS: {
		int m = 0;
		char *dp = dst;
		const char *dend = dst+dstcapa;
		const char *sp = src;
		do {
			switch (m) {
			case 0: // First
				if (*sp == '=') {
					m = 100;
					*dp++ = *sp;
					*dp++ = wrpch;
				} else {
					*dp++ = *sp;
				}
			break;
			case 100: // =
				if (*sp == '\0') {
					*dp++ = wrpch;
					*dp = '\0';
					goto done;
				} else {
					*dp++ = *sp;
				}
			break;
			}
		} while (*sp++ && dp < dend);
	} break;
	}

	done: {
		struct cap_arg *wrparg = cap_argnew();
		if (!wrparg) {
			free(dst);
			return NULL;
		}

		dst[dstcapa-1] = '\0';
		wrparg->type = self->type;
		wrparg->value = dst;

		return wrparg;
	}
}

/*******
* args *
*******/

struct cap_args {
	struct cap_arg **args;
	int len;
	int capa;
};

void
cap_argsdel(struct cap_args *self) {
	if (self) {
		for (int i = 0; i < self->len; ++i) {
			cap_argdel(self->args[i]);
		}
		free(self->args);
		free(self);
	}
}

struct cap_args *
cap_argsnew(void) {
	struct cap_args *self = calloc(1, sizeof(*self));

	self->capa = NCAPA;
	self->args = calloc(self->capa+1, sizeof(struct cap_arg *)); // +1 for final nul

	return self;
}

struct cap_args *
cap_argsclear(struct cap_args *self) {
	for (int i = 0; i < self->len; ++i) {
		free(self->args[i]);
		self->args[i] = NULL;
	}
	return self;
}

struct cap_args *
cap_argsresize(struct cap_args *self, int newcapa) {
	struct cap_arg **tmp = realloc(self->args, sizeof(struct cap_arg *) * newcapa + 1);
	if (!tmp) {
		return NULL;
	}

	self->capa = newcapa;
	self->args = tmp;

	return self;
}

struct cap_args *
cap_argsmove(struct cap_args *self, struct cap_arg *arg) {
	if (self->len >= self->capa) {
		if (!cap_argsresize(self, self->capa*2)) {
			perror("resize args");
		}
	}

	self->args[self->len++] = arg;

	return self;
}

struct cap_args *
cap_argsparse(struct cap_args *self, int argc, char *argv[]) {
	cap_argsclear(self);
	for (int i = 0; i < argc; ++i) {
		struct cap_arg *arg = cap_argnew();
		cap_argparse(arg, argv[i]);
		cap_argsmove(self, arg);
	}
	return self;
}

void
cap_argsshow(const struct cap_args *self, FILE *fout) {
	for (int i = 0; i < self->len; ++i) {
		cap_argshow(self->args[i], fout);
	}
}

int
cap_argslen(const struct cap_args *self) {
	return self->len;
}

int
cap_argscapa(const struct cap_args *self) {
	return self->capa;
}

const struct cap_arg *
cap_argsgetc(const struct cap_args *self, int idx) {
	if (idx >= self->len || idx < 0) {
		return NULL;
	}
	return self->args[idx];
}

#if defined(_TEST_ARGS)
int
main(int argc, char *argv[]) {
	struct cap_args *args = cap_argsnew();

	cap_argsparse(args, argc-1, argv+1);
	// cap_argsshow(args, stderr);

	for (int i = 0; i < cap_argslen(args); ++i) {
		const struct cap_arg *arg = cap_argsgetc(args, i);
		struct cap_arg *wrp = cap_argwrapvalue(arg, '"');
		printf("wrp->value[%s]\n", cap_argvaluec(wrp));
		cap_argdel(wrp);
		// cap_argshow(arg, stderr);
/*		switch (cap_argtype(arg)) {
		case ARG_ARGUMENT:
			cap_argshow(arg, stderr);
		break;
		}
*/	}

	cap_argsdel(args);
	return 0;
}
#endif
