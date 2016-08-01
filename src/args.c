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
argdel(struct cap_arg *self) {
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
cap_argparse(struct cap_arg *self, const char *src) {
	if (self->value) {
		free(self->value);
	}

	self->value = strdup(src);
	if (!self->value) {
		return NULL;
	}

	return self;
}

void
cap_argshow(const struct cap_arg *self, FILE *fout) {
	fprintf(fout, "<cap_arg value=\"%s\">\n", self->value);
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
			argdel(self->args[i]);
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

#if defined(_TEST_ARGS)
int
main(int argc, char *argv[]) {
	struct cap_args *args = cap_argsnew();

	cap_argsparse(args, argc, argv);
	cap_argsshow(args, stderr);

	cap_argsdel(args);
	return 0;
}
#endif
