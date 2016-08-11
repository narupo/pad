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
	return self;
}

struct cap_cl *
cap_clparse(struct cap_cl *self, const char *drtcl) {
	int m = 0;
	const char *p = drtcl;


	do {
		int c = *p++;

		switch (m) {
		case 0: // First
		break;
		}
	} while (*p);

	return self;
}

void
cap_clshow(const struct cap_cl *self, FILE *fout) {
	for (int i = 0; i < self->len; ++i) {
		fprintf(fout, "<arg index=\"%d\" value=\"%s\">\n", i, self->arr[i]);
	}
}

#if defined(_TEST_CL)
int
main(int argc, char* argv[]) {
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
#endif
