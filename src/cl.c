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

char **
cap_clescdel(struct cap_cl *self) {
	if (!self) {
		return NULL;
	}
	char **arr = self->arr;
	free(self);
	return arr;
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
	self->arr[self->len] = NULL;
	return self;
}

void
cap_clclear(struct cap_cl *self) {
	for (int i = 0; i < self->len; ++i) {
		free(self->arr[i]);
		self->arr[i] = NULL;
	}
	self->len = 0;
}

int
cap_cllen(const struct cap_cl *self) {
	return self->len;
}

int
cap_clcapa(const struct cap_cl *self) {
	return self->capa;
}

const char *
cap_clgetc(const struct cap_cl *self, int idx) {
	if (idx < 0 || idx >= self->len) {
		return NULL;
	}	
	return self->arr[idx];
}

/*********
* string *
*********/

struct cl_string {
	int capa;
	int len;
	char *arr;
};

static void
cl_strdel(struct cl_string *self) {
	if (self) {
		free(self->arr);
		free(self);
	}
}

static struct cl_string *
cl_strnew(void) {
	struct cl_string *self = calloc(1, sizeof(*self));
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

static struct cl_string *
cl_strresize(struct cl_string *self, int newcapa) {
	char *tmp = realloc(self->arr, sizeof(char) * newcapa + sizeof(char));
	if (!tmp) {
		perror("realloc");
		exit(1);
	}

	self->capa = newcapa;
	self->arr = tmp;
	self->arr[self->capa] = '\0';

	return self;
}

static struct cl_string *
cl_strpush(struct cl_string *self, char c) {
	if (self->len >= self->capa) {
		if (!cl_strresize(self, self->capa*2)) {
			perror("cl_strresize");
			exit(1);
		}
	}
	self->arr[self->len++] = c;
	self->arr[self->len] = '\0';
	return self;
}

static struct cl_string *
cl_strset(struct cl_string *self, const char *src) {
	int srclen = strlen(src);

	if (srclen >= self->len) {
		if (!cl_strresize(self, srclen)) {
			return NULL;
		}
	}

	self->len = srclen;

	for (int i = 0; i < srclen; ++i) {
		self->arr[i] = src[i];
	}
	self->arr[srclen] = '\0';
	return self;
}

static const char *
cl_strgetc(const struct cl_string *self) {
	return self->arr;
}

static void
cl_strclear(struct cl_string *self) {
	self->len = 0;
	self->arr[self->len] = '\0';
}

static int
cl_strlen(const struct cl_string *self) {
	return self->len;
}

static struct cl_string *
cl_strapp(struct cl_string *self, const char *src) {
	int srclen = strlen(src);

	if (self->len + srclen >= self->capa-1) {
		if (!cl_strresize(self, (self->len + srclen) * 2)) {
			return NULL;
		}
	}

	for (const char *sp = src; *sp; ++sp) {
		self->arr[self->len++] = *sp;
	}
	self->arr[self->len] = '\0';

	return self;
}

static bool
isnormch(int c) {
	return isalnum(c) || c == '-' || c == '_';
}

static bool
ismetach(int c) {
	return strchr("<>();&|", c) != NULL;
}

static void
escapecpy(struct cl_string *dst, const struct cl_string *src, int opts) {
	const char *srcval = cl_strgetc(src);
	int m = 0;
	for (const char *p = srcval; *p; ++p) {
		if (*p == '\\') {
			if (*++p == '\0') {
				break;
			}
			cl_strpush(dst, '\\');
			cl_strpush(dst, *p);
			continue;
		}

		if (opts & CL_DEBUG) {
			printf("esc: m[%d] c[%c]\n", m, *p);
		}

		switch (m) {
		case 0: // First
			if (ismetach(*p)) {
				cl_strpush(dst, '\\');
				cl_strpush(dst, *p);
			} else if (*p == '"') {
				m = 10;
				cl_strpush(dst, *p);
			} else if (*p == '\'') {
				m = 20;
				cl_strpush(dst, '\\');
				cl_strpush(dst, *p);
			} else {
				cl_strpush(dst, *p);
			}
		break;
		case 10: // ""
			if (*p == '"') {
				m = 0;
				cl_strpush(dst, *p);
			} else {
				cl_strpush(dst, *p);
			}
		break;
		case 20: // ''
			if (*p == '\'') {
				m = 0;
				if (opts & CL_WRAP) {
					cl_strpush(dst, '\\');
				}
				cl_strpush(dst, *p);
			} else {
				cl_strpush(dst, *p);
			}
		break;
		}
	}
}

static void
validatepush(struct cap_cl *cl, struct cl_string *src, int opts) {
	if (!cl_strlen(src)) {
		return;
	}

	struct cl_string *dst = cl_strnew();

	if (opts & CL_WRAP) {
		cl_strpush(dst, '\'');
	}

	if (opts & CL_ESCAPE) {
		escapecpy(dst, src, opts);
	} else {
		cl_strset(dst, cl_strgetc(src));
	}

	if (opts & CL_WRAP) {
		cl_strpush(dst, '\'');
	}

// printf("dst[%s]\n", cl_strgetc(dst));//debug
	cap_clpush(cl, cl_strgetc(dst));
	cl_strdel(dst);
	cl_strclear(src);
}

struct cap_cl *
cap_clparsestropts(struct cap_cl *self, const char *drtsrc, int opts) {
	int m = 0;
	const char *p = drtsrc;
	struct cl_string *tmp = cl_strnew();
	opts = (opts < 0 ? CL_ESCAPE : opts);

	cap_clclear(self);

	do {
		int c = *p;
		if (c == '\0') {
			validatepush(self, tmp, opts);
			break;
		}

		if (opts & CL_DEBUG) {
			printf("m[%d] c[%c]\n", m, c);
		}

		if (c == '\\') {
			if (*++p == '\0') {
				cl_strdel(tmp);
				return NULL;
			}
			cl_strpush(tmp, c);
			cl_strpush(tmp, *p);
			continue;
		}

		switch (m) {
		case 0: // First
			if (isspace(c)) {
				;
			} else if (c == '-') {
				m = 100;
				cl_strpush(tmp, c);
			} else if (c == '"') {
				m = 10;
			} else if (c == '\'') {
				m = 20;
			} else {
				m = 30;
				cl_strpush(tmp, c);
			}
		break;
		case 10: // "arg"
			if (c == '"') {
				m = 0;
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 20: // 'arg'
			if (c == '\'') {
				m = 0;
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 30: // arg
			if (isspace(c)) {
				m = 0;
				validatepush(self, tmp, opts);
			} else if (c == '"' || c == '\'') {
				; // Ignore
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 100: // -
			if (c == '-') {
				m = 150;
				cl_strpush(tmp, c);
			} else if (isnormch(c)) {
				m = 110;
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 110: // -?
			if (isnormch(c)) {
				cl_strpush(tmp, c);
			} else if (c == '=') {
				m = 200;
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 150: // --
			if (isnormch(c)) {
				m = 160;
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 160: // --?
			if (isnormch(c)) {
				cl_strpush(tmp, c);
			} else if (c == '=') {
				m = 200;				
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 200: // -?= or --?=
			if (isspace(c)) {
				m = 0;
				validatepush(self, tmp, opts);
			} else if (c == '"') {
				m = 210;
				cl_strpush(tmp, c);
			} else if (c == '\'') {
				m = 220;
				cl_strpush(tmp, c);
			} else {
				m = 230;
				cl_strpush(tmp, c);
			}
		break;
		case 210: // -?="arg" or --?="arg"
			if (c == '"') {
				m = 0;
				cl_strpush(tmp, c);
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 220: // -?='arg' or --?='arg'
			if (c == '\'') {
				m = 0;
				cl_strpush(tmp, c);
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 230: // -?=arg or --?=arg
			if (isspace(c)) {
				m = 0;
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		}
	} while (*p++);

	cl_strdel(tmp);
	return self;
}

struct cap_cl *
cap_clparsestr(struct cap_cl *self, const char *drtcl) {
	return cap_clparsestropts(self, drtcl, -1);
}

struct cap_cl *
cap_clparseargvopts(struct cap_cl *self, int argc, char *argv[], int opts) {
	struct cl_string *line = cl_strnew();
	for (int i = 0; i < argc; ++i) {
		cl_strpush(line, '\'');
		for (const char *p = argv[i]; *p; ++p) {
			if (*p == '\\') {
				if (*++p == '\0') {
					break;
				}
				cl_strpush(line, '\\');
				cl_strpush(line, *p);
			} else if (*p == '\'') {
				cl_strpush(line, '\\');
				cl_strpush(line, *p);
			} else {
				cl_strpush(line, *p);
			}
		}
		cl_strpush(line, '\'');
		cl_strpush(line, ' ');
	}
	self = cap_clparsestropts(self, cl_strgetc(line), opts);
	cl_strdel(line);
	return self;	
}

struct cap_cl *
cap_clparseargv(struct cap_cl *self, int argc, char *argv[]) {
	return cap_clparseargvopts(self, argc, argv, -1);
}

void
cap_clshow(const struct cap_cl *self, FILE *fout) {
	for (int i = 0; i < self->len; ++i) {
		// fprintf(fout, "<arg index=\"%d\" value=\"%s\">\n", i, self->arr[i]);
		fprintf(fout, "[%d] = [%s]\n", i, self->arr[i]);
	}
}

/*******
* test *
*******/

#if defined(_TEST_CL)
#include <stdio.h>

static void
freeargv(char *argv[]) {
	for (char **a = argv; *a; ++a) {
		free(*a);
	}
	free(argv);
}

static void
showargv(char *argv[]) {
	for (char **a = argv; *a; ++a) {
		puts(*a);
	}
}

static void 
die(const char *fmt) {
	perror(fmt);
	exit(1);
}

static int
test_parsestr(int argc, char *argv[]) {
	if (argc < 2) {
		die("need argv");
	}

	struct cap_cl *cl = cap_clnew();
	cap_clparsestropts(cl, argv[1], CL_DEBUG | CL_ESCAPE);
	cap_clshow(cl, stderr);
	cap_cldel(cl);
	printf("source [%s]\n", argv[1]);
	return 0;
}

static int
test_parseargv(int argc, char *argv[]) {
	struct cap_cl *cl = cap_clnew();
	cap_clparseargvopts(cl, argc, argv, CL_DEBUG | CL_ESCAPE);
	cap_clshow(cl, stderr);
	cap_cldel(cl);
	return 0;
}

static int
test_execv(int argc, char *argv[]) {
	struct cap_cl *cl = cap_clnew();
	showargv(argv);
	cap_clparseargvopts(cl, argc, argv, CL_DEBUG | CL_ESCAPE);
	char **av = cap_clescdel(cl);
	showargv(av);

	switch (fork()) {
	case -1:
		die("fork");
	break;
	case 0:
		execv("/home/narupo/src/bottle/src/bin/args", av);
		perror("execv");
		freeargv(av);
		_exit(1);
	break;
	default:
		wait(NULL);
		freeargv(av);
		exit(0);
	break;
	}

	return 0;
}

int
main(int argc, char *argv[]) {
	static const struct cmd {
		const char *name;
		int (*func)(int, char**);
	} cmds[] = {
		{"parsestr", test_parsestr},
		{"parseargv", test_parseargv},
		{"execv", test_execv},
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
