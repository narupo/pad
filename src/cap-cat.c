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
	char **argv;
	bool ishelp;
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
 * Catenate fin to fout
 * 
 * @param[in] *fout destination stream
 * @param[in] *fin source stream
 * 
 * @return success to number of zero
 * @return failed to not a number of zero
 */
static int
catstream(FILE *fout, FILE *fin) {
	if (!cap_fcopy(fout, fin)) {
		return 1;
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
catfile(FILE *fout, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return false;
	}

	if (catstream(fout, fin) != 0) {
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
		"    Catenate files.\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help    show usage\n"
		"\n"
	);
	exit(1);
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
	*self = (struct opts){};

	// Parse options
	static struct option longopts[] = {
		{"help", no_argument, 0, 'h'},
		{},
	};
	optind = 0;

	for (;;) {
		int optsindex;
		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 0: /* Long option only */ break;
		case 'h': self->ishelp = true; break;
		case '?':
		default: return NULL; break;
		}
	}

	if (argc < optind) {
		return NULL;
	}

	self->argc = argc-optind+1;
	self->argv = argv;

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

	if (opts->argc < 2) {
		return catstream(stdout, stdin);
	}

	char cdpath[FILE_NPATH];
	if (!cap_envget(cdpath, sizeof cdpath, "CAP_VARCD")) {
		cap_die("need environment variable of cd");
	}

	int ret = 0;
	for (int i = 1; i < opts->argc; ++i) {
		const char *name = opts->argv[i];
		
		char path[FILE_NPATH];
		if (!makepath(path, sizeof path, cdpath, name)) {
			++ret;
			cap_error("failed to make path by '%s'", name);
			continue;
		}

		if (!catfile(stdout, path)) {
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
