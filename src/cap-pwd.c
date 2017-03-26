/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-pwd.h"

struct opts {
    bool ishelp;
    bool isnorm;
};

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"normalize", no_argument, 0, 'n'},
        {},
    };
    const char *shortopts = "hn";

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->ishelp = true; break;
        case 'n': self->isnorm = true; break;
        case '?':
        default: perror("Unknown option"); break;
        }
    }

    if (argc < optind) {
        perror("Failed to parse option");
        return false;
    }

    return true;
}


int
main(int argc, char *argv[]) {
    struct opts opts;
    if (!optsparse(&opts, argc, argv)) {
        cap_error("failed to parse option");
        return 1;
    }

	char cd[FILE_NPATH];
	if (!cap_envget(cd, sizeof cd, "CAP_VARCD")) {
		cap_error("need environment variable of cd");
		return 2;
	}

    char home[FILE_NPATH];
    if (!cap_envget(home, sizeof home, "CAP_VARHOME")) {
        cap_error("need environment variable of home");
        return 3;
    }

    if (opts.isnorm) {
    	printf("%s\n", cd);
    } else {
        int32_t homelen = strlen(home);
        int32_t cdlen = strlen(cd);
        if (cdlen-homelen < 0) {
            cap_error("invalid cd \"%s\" or home \"%s\"", cd, home);
            return 4;
        }
        if (cdlen-homelen == 0) {
            printf("/\n");
        } else {
            const char *p = cd + homelen;
            printf("%s\n", p);
        }
    }

	return 0;
}
