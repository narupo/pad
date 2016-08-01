#include "args.h"

enum {
	NCAPA = 4,
};

// arg

struct cap_arg {
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
argnew(void) {
	struct cap_arg *self = calloc(1, sizeof(*self));
	return self;
}

struct cap_arg *
argparse(struct cap_arg *self, const char *src) {
	return self;
}

// args

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
cap_argsmoveb(struct cap_args *self, struct cap_arg *arg) {

}

struct cap_args *
cap_argsparse(struct cap_args *self, int argc, const char *argv[]) {
	cap_argsclear(self);
	for (int i = 0; i < argc; ++i) {
		struct cap_arg *arg = argnew();
		argparse(arg, argv[i]);
		cap_argsmoveb(self, arg);
	}
	return self;
}

#if defined(_TEST_ARGS)
int
main(int argc, char *argv[]) {
	struct cap_args *args = cap_argsnew()

	cap_argsparse(args, argc, argv);

	cap_argsdel(args);
	return 0;
}
#endif
