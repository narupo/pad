/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-ls.h"

struct opts {
    bool ishelp;
    bool isall;
} opts;

static void
arrdump(const struct cap_array *arr, FILE *fout) {
	for (int i = 0; i < cap_arrlen(arr); ++i) {
		fprintf(fout, "%s\n", cap_arrgetc(arr, i));
	}
	fflush(fout);
}

static bool
isdotfile(const char *fname) {
    if (opts.isall) {
        return false;
    }
    
    if (strcmp(fname, "..") == 0 ||
        fname[0] == '.') {
        return true;
    }

    return false;
}

static struct cap_array *
dir2array(struct cap_dir *dir) {
	struct cap_array *arr = cap_arrnew();
	if (!arr) {
		return NULL;
	}

	for (struct cap_dirnode *nd; (nd = cap_dirread(dir)); ) {
		const char *name = cap_dirnodename(nd);
        if (!isdotfile(name)) {
            cap_arrpush(arr, name);
            
        }
		cap_dirnodedel(nd);
	}

	return arr;
}

static int
capls(const char *path) {
	if (isoutofhome(path)) {
		cap_error("'%s' is out of home", path);
		return 1;
	}
	
	struct cap_dir *dir = cap_diropen(path);
	if (!dir) {
		cap_error("failed to open directory %s", path);
		return 1;
	}

	struct cap_array *arr = dir2array(dir);
	if (!arr) {
		cap_error("failed to read directory %s", path);
		return 1;
	}

	cap_arrsort(arr);
	arrdump(arr, stdout);
	cap_arrdel(arr);

	if (cap_dirclose(dir) < 0) {
		cap_error("failed to close directory %s", path);
		return 1;
	}

	return 0;
}

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"all", no_argument, 0, 'a'},
        {},
    };
    
    *self = (struct opts){};
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "ha", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->ishelp = true; break;
        case 'a': self->isall = true; break;
        case '?':
        default:
            return false;
            break;
        }
    }

    if (argc < optind) {
        return false;
    }

    return true;
}

static void
usage(void) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "   -h, --help    show usage.\n"
        "   -a, --all     show all files\n"
        "\n"
    , getenv("CAP_PROCNAME"));
    exit(0);
}

int
main(int argc, char *argv[]) {
    cap_envsetf("CAP_PROCNAME", "cap ls");

    if (!optsparse(&opts, argc, argv)) {
        cap_die("failed to parse options");
    }

    if (opts.ishelp) {
        usage();
    }

	char cd[FILE_NPATH];
	if (!cap_envget(cd, sizeof cd, "CAP_VARCD")) {
		cap_die("need environment variable of cd");
	}

    char home[FILE_NPATH];
    if (!cap_envget(home, sizeof home, "CAP_VARHOME")) {
        cap_die("need environment variable of home");
    }

	if (optind-argc == 0) {
		capls(cd);
	} else {
		char path[FILE_NPATH];

        for (int i = optind; i < argc; ++i) {
            const char *arg = argv[i];
            const char *org = (arg[0] == '/' ? home : cd);
			cap_fsolvefmt(path, sizeof path, "%s/%s", org, arg);
			capls(path);
		}
	}

	return 0;
}
