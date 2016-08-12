#include "cl.h"

struct cap_cl {
	int capa;
	int len;
	char **arr;
};

void
cap_cldel(struct cap_cl *self) {
	if (self) {
		for (int i = 0; i < self->len; ++i) {
			free(self->arr[i]);
		}
		free(self->arr);
		free(self);
	}
}

struct cap_cl *
cap_clnew(void) {
	struct cap_cl *self = calloc(1, sizeof(*self));
	if (!self) {
		return NULL;
	}

	self->len = 0;
	self->capa = 4;
	self->arr = calloc(self->capa+1, sizeof(char*));
	if (!self->arr) {
		return NULL;
	}

	return self;
}

struct cap_cl *
cap_clresize(struct cap_cl *self, int newcapa) {
	if (newcapa <= self->capa) {
		return NULL; // Unsupported
	}

	char **tmp = realloc(self->arr, sizeof(char*) * newcapa + sizeof(char*));
	if (!tmp) {
		return NULL;
	}

	self->arr = tmp;
	self->capa = newcapa;

	return self;
}

struct cap_cl *
cap_clpush(struct cap_cl *self, const char *str) {
	if (self->len >= self->capa) {
		if (!cap_clresize(self, self->capa*2)) {
			return NULL;
		}
	}
	self->arr[self->len++] = strdup(str);
	return self;
}

struct string {
	int capa;
	int len;
	char *arr;
};

static void
strdel(struct string *self) {
	if (self) {
		free(self->arr);
		free(self);
	}
}

static struct string *
strnew(void) {
	struct string *self = calloc(1, sizeof(*self));
	if (!self) {
		perror("calloc");
		exit(1);
	}

	self->capa = 4;
	self->arr = calloc(self->capa+1, sizeof(char));
	if (!self->arr) {
		perror("calloc");
		exit(1);
	}

	return self;
}

static struct string *
strresize(struct string *self, int newcapa) {
	if (newcapa <= self->capa) {
		perror("TODO");
		exit(1);
	}

	char *tmp = realloc(self->arr, sizeof(char) * newcapa + sizeof(char));
	if (!tmp) {
		perror("realloc");
		exit(1);
	}

	self->capa = newcapa;
	self->arr = tmp;

	return self;
}

static struct string *
strpush(struct string *self, char c) {
	if (self->len >= self->capa) {
		if (!strresize(self, self->capa*2)) {
			perror("strresize");
			exit(1);
		}
	}
	self->arr[self->len++] = c;
	self->arr[self->len] = '\0';
	return self;
}

static const char *
strgetc(struct string *self) {
	return self->arr;
}

static void
strclear(struct string *self) {
	self->len = 0;
	self->arr[self->len] = '\0';
}

static bool
isnormch(int c) {
	return isalnum(c) || c == '-' || c == '_';
}

struct cap_cl *
cap_clparse(struct cap_cl *self, const char *drtcl) {
	int m = 0;
	const char *p = drtcl;
	struct string *tmp = strnew();

	do {
		int c = *p;
		if (c == '\0') {
			c = ' ';
		}

		printf("m[%d] c[%c]\n", m, c);

		switch (m) {
		case 0: // First
			if (isblank(c)) {
				;
			} else if (c == '-') {
				m = 100;
				strpush(tmp, c);
			} else {
				m = 10;
				strpush(tmp, c);
			}
		break;
		case 10: // arg
			if (isblank(c)) {
				m = 0;
				cap_clpush(self, strgetc(tmp));
				strclear(tmp);
			} else {
				strpush(tmp, c);
			}
		break;
		case 100: // -
			if (c == '-') {
				m = 150;
				strpush(tmp, c);
			} else if (isnormch(c)) {
				m = 110;
				strpush(tmp, c);
			} else {
				m = 0;
				cap_clpush(self, strgetc(tmp));
				strclear(tmp);
			}
		break;
		case 110: // -?
			if (isnormch(c)) {
				strpush(tmp, c);
			} else {
				m = 0;
				cap_clpush(self, strgetc(tmp));
				strclear(tmp);
			}
		break;
		case 150: // --
			if (isnormch(c)) {
				m = 160;
				strpush(tmp, c);
			} else {
				m = 0;
				cap_clpush(self, strgetc(tmp));
				strclear(tmp);
			}
		break;
		case 160: // --?
			if (isnormch(c)) {
				strpush(tmp, c);
			} else {
				m = 0;
				cap_clpush(self, strgetc(tmp));
				strclear(tmp);
			}
		break;
		}
	} while (*p++);

	strdel(tmp);
	return self;
}

void
cap_clshow(const struct cap_cl *self, FILE *fout) {
	for (int i = 0; i < self->len; ++i) {
		fprintf(fout, "<arg index=\"%d\" value=\"%s\">\n", i, self->arr[i]);
	}
}

#if defined(_TEST_CL)
#include <stdio.h>

static int
test_a(int argc, char *argv[]) {
	if (argc < 2) {
		perror("need cl");
		return 1;
	}

	struct cap_cl *cl = cap_clnew();
	cap_clparse(cl, argv[1]);
	cap_clshow(cl, stderr);
	cap_cldel(cl);
	return 0;
}

int
main(int argc, char *argv[]) {
	static const struct cmd {
		const char *name;
		int (*func)(int, char**);
	} cmds[] = {
		{"a", test_a},
		{},
	};

	if (argc < 2) {
		fprintf(stderr,
			"Usage: %s [command]\n"
			"\n"
			"The commands are:\n\n"
		, argv[0]);
		for (const struct cmd *p = cmds; p->name; ++p) {
			fprintf(stderr, "    %s\n", p->name);
		}
		fprintf(stderr, "\n");
		return 1;
	}

	for (const struct cmd *p = cmds; p->name; ++p) {
		if (!strcmp(p->name, argv[1])) {
			return p->func(argc-1, argv+1);
		}
	}

	fprintf(stderr, "Not found command of '%s'\n", argv[1]);
	return 1;
}
#endif
