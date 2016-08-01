#include "args.h"

/*********
* number *
*********/

enum {
	ARGS_NCAPA = 4,
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

char *
cap_argescdel(struct cap_arg *self) {
	if (!self) {
		return NULL;
	}

	char *value = self->value;
	free(self);

	return value;
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

const char *
cap_argwrapvalue(struct cap_arg *self, int wrpch) {
	if (!self->value) {
		return NULL;
	}

	const char *src = self->value;
	int srclen = strlen(src);
	int dstcapa = srclen+2+1; // (wrpch * 2) + '\0'
	char *dst = calloc(dstcapa, sizeof(self->value[0])); 
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
		dst[dstcapa-1] = '\0';
		free(self->value);
		self->value = dst;
		return self->value;
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

char **
cap_argsescdel(struct cap_args *self) {
	if (!self) {
		return NULL;
	}

	char **argv = calloc(self->len+1, sizeof(char*)); // +1 for final nul
	if (!argv) {
		return NULL;
	}

	for (int i = 0; i < self->len; ++i) {
		argv[i] = cap_argescdel(self->args[i]);
	}
	free(self->args);
	free(self);

	return argv;
}

struct cap_args *
cap_argsnew(void) {
	struct cap_args *self = calloc(1, sizeof(*self));

	self->capa = ARGS_NCAPA;
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

struct cap_arg *
cap_argsget(struct cap_args *self, int idx) {
	if (idx >= self->len || idx < 0) {
		return NULL;
	}
	return self->args[idx];
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

	cap_argsparse(args, argc, argv);
	// cap_argsshow(args, stderr);

	for (int i = 1; i < cap_argslen(args); ++i) {
		struct cap_arg *arg = cap_argsget(args, i);
		const char *value = cap_argwrapvalue(arg, '"');
		printf("value[%s]\n", value);
		// cap_argshow(arg, stderr);
/*		switch (cap_argtype(arg)) {
		case ARG_ARGUMENT:
			cap_argshow(arg, stderr);
		break;
		}
*/	}

	int newargc = cap_argslen(args);
	char **newargv = cap_argsescdel(args);
	for (int i = 0; i < newargc; ++i) {
		printf("[%d] = [%s]\n", i, newargv[i]);
		free(newargv[i]);
	}
	free(newargv);

	return 0;
}
#endif
