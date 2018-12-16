/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-cat.h"

struct opts {
	int argc;
	int optind;
	char **argv;
	bool ishelp;
	int indent;
	int tabspaces;
	bool istab;
};

/**
 * Make file path for catenate
 * 
 * @param[in] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *cdpath string of cd path
 * @param[in] *name string of name
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
static char *
makepath(char *dst, size_t dstsz, const char *cdpath, const char *name) {
	if (!cap_fsolvefmt(dst, dstsz, "%s/%s", cdpath, name)) {
		return NULL;
	}

	if (isoutofhome(dst)) {
		return NULL;
	}

	if (cap_fisdir(dst)) {
		return NULL;
	}

	return dst;
}

/**
 * Set indent characters.
 *
 * @param[in] *opts options
 * @param[out] *buf buffer
 * @param[in] bufsize buffer size
 *
 * @return success to true
 * @return failed to false
 */
static bool
setindent(const struct opts *opts, char *buf, size_t bufsize) {
	if (opts->istab) {
		buf[0] = '\t';
		buf[1] = '\0';
	} else {
		if (opts->tabspaces >= bufsize-1) {
			return false;
		}

		memset(buf, ' ', opts->tabspaces);
		buf[opts->tabspaces] = '\0';
	}

	return true;
}

/**
 * Catenate fin to fout
 * 
 * @param[in] *fout destination stream
 * @param[in] *fin source stream
 * 
 * @return success to number of zero
 * @return failed to not a number of zero
 */
static int
catstream(const struct opts *opts, FILE *fout, FILE *fin) {
	int m = 0;

	for (;;) {
		int c = fgetc(fin);
		if (c == EOF) {
			break;
		}

		switch (m) {
		case 0: { // Indent mode
			char str[100] = {0};
			if (!setindent(opts, str, sizeof str)) {
				return 1;
			}

			for (int i = 0; i < opts->indent; ++i) {
				fprintf(fout, "%s", str);
			}

			fputc(c, fout);
			if (c != '\n') {
				m = 1;
			}
		} break;
		case 1: { // Stream mode
			fputc(c, fout);
			if (c == '\n') {
				m = 0;
			}
		} break;
		}
	}

	return 0;
}

/**
 * Catenate file content to fout
 * 
 * @param[in] *fout destination buffer
 * @param[in] *path file path
 * 
 * @return success to true
 * @return failed to false
 */
static bool
catfile(const struct opts *opts, FILE *fout, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return false;
	}

	if (catstream(opts, fout, fin) != 0) {
		fclose(fin);
		return false;
	}
	
	if (fclose(fin) < 0) {
		return false;
	}

	return true;
}

/**
 * Show usage and exit from proccess
 * 
 */
static void
usage(void) {
	fprintf(stderr,
		"Usage: cap cat [options] [files]\n"
		"\n"
		"    Concatenate files.\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help       show usage.\n"
		"    -i, --indent     indent spaces.\n"
		"    -T, --tabspaces  number of tab spaces.\n"
		"    -t, --tab        tab indent mode.\n"
		"\n"
		"Examples:\n"
		"\n"
		"    $ cap cat f - g\n"
		"    $ cap cat\n"
		"\n"
	);
	exit(0);
}

/**
 * Parse options
 * 
 * @param[in] *self   
 * @param[in] argc    
 * @param[in] *argv[] 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct opts *
optsparse(struct opts *self, int argc, char *argv[]) {
	// Parse options
	static struct option longopts[] = {
		{"help", no_argument, 0, 'h'},
		{"indent", required_argument, 0, 'i'},
		{"tabspaces", required_argument, 0, 'T'},
		{"tab", no_argument, 0, 't'},
		{},
	};

	*self = (struct opts){
		.ishelp = false,
		.indent = 0,
		.tabspaces = 4,
		.istab = false,
	};
	opterr = 0;
	optind = 0;

	for (;;) {
		int optsindex;
		int cur = getopt_long(argc, argv, "hi:T:t", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 0: /* Long option only */ break;
		case 'h': self->ishelp = true; break;
		case 'i': self->indent = atoi(optarg); break;
		case 'T': self->tabspaces = atoi(optarg); break;
		case 't': self->istab = true; break;
		case '?':
		default: return NULL; break;
		}
	}

	if (argc < optind) {
		return NULL;
	}

	self->argc = argc;
	self->argv = argv;
	self->optind = optind;

	return self;
}

/**
 * Run cap-cat
 * 
 * @param[in] *opts options
 * 
 * @return success to number of zero
 * @return failed to not a number of zero
 */
static int
run(struct opts *opts) {
	if (opts->ishelp) {
		usage();
	}

	if (opts->argc - opts->optind + 1 < 2) {
		return catstream(opts, stdout, stdin);
	}

	const char *scope = getenv("CAP_SCOPE");
	if (!scope) {
		cap_error("not found scope in environment");
		return 1;
	}

	char cdpath[FILE_NPATH];
	if (strcasecmp(scope, "local") == 0) {
		if (!cap_envget(cdpath, sizeof cdpath, "CAP_VARCD")) {
			cap_die("need environment variable of cd");
		}
	} else if (strcasecmp(scope, "global") == 0) {
		if (!cap_envget(cdpath, sizeof cdpath, "CAP_VARHOME")) {
			cap_die("need environment variable of home");
		}
	} else {
		cap_error("invalid scope \"%s\"", scope);
		return 1;
	}

	int ret = 0;
	for (int i = opts->optind; i < opts->argc; ++i) {
		const char *name = opts->argv[i];

		if (strcmp(name, "-") == 0) {
			catstream(opts, stdout, stdin);
			continue;
		}
		
		char path[FILE_NPATH];
		if (!makepath(path, sizeof path, cdpath, name)) {
			++ret;
			cap_error("failed to make path by '%s'", name);
			continue;
		}

		if (!catfile(opts, stdout, path)) {
			++ret;
			cap_error("failed to catenate of '%s'", path);
			continue;
		}
	}

	return ret;
}

/**
 * Main routine
 *
 */
int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap cat");

	struct opts opts;
	if (!optsparse(&opts, argc, argv)) {
		cap_die("failed to parse options");
	}

	return run(&opts);
}
