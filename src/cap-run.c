/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-run.h"

enum {
	NSCRIPTNAME = 100,
	NCMDLINE = 256
};

static char *
readscriptline(char *dst, size_t dstsz, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return NULL;
	}

	char tmp[dstsz];
	cap_fgetline(tmp, sizeof tmp, fin);

	const char *needle = "!";
	char *at = strstr(tmp, needle);
	if (!at) {
		fclose(fin);
		return NULL;
	}

	snprintf(dst, dstsz, "%s", at + strlen(needle));

	if (fclose(fin) < 0) {
		return NULL;
	}

	return dst;
}

struct opts {
    bool ishelp;
};

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"fname", required_argument, 0, 'f'},
        {},
    };

    extern int opterr;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "hf:", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': /* Help */ break;
        case '?':
        default: cap_error("Unknown option"); break;
        }
    }

    if (argc < optind) {
        cap_error("Failed to parse option");
        return false;
    }

    return true;
}

static struct cap_array *
fixargs(int argc, char *argv[]) {
    const char *optslist[] = {
        "-h",
        "--help",
        "-g",
        "--global",
        NULL,
    };
    int m = 0;

    struct cap_array *args = cap_arrnew();
    if (!args) {
        return NULL;
    }

    for (int i = 0; i < argc; ++i) {
        const char *arg = argv[i];
        switch (m) {
        case 0:
            if (strcmp(arg, "run") == 0) {
                cap_arrpush(args, arg);
                m = 1;
            }
        break;
        case 1: {
            bool matched = false;
            for (int i = 0; optslist[i]; ++i) {
                if (strcmp(optslist[i], arg) == 0) {
                    matched = true;
                    break; // through
                }
            }
            if (!matched) {
                cap_arrpush(args, arg);
                m = 2;                
            }
        } break;
        case 2:
            cap_arrpush(args, arg);
        break;
        }
    }

    return args;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap run");

	if (argc < 2) {
		cap_error("need script name");
		return 1;
	}

	char varcd[FILE_NPATH];
	if (!cap_envget(varcd, sizeof varcd, "CAP_VARCD")) {
		cap_error("need environment variable of cd");
		return 1;
	}

	char spath[FILE_NPATH]; // Script path
	cap_fsolvefmt(spath, sizeof spath, "%s/%s", varcd, argv[1]);
	if (isoutofhome(spath)) {
		cap_die("invalid script. '%s' is out of home.", spath);
	}

	char exesname[NSCRIPTNAME]; // Execute script name in file
	readscriptline(exesname, sizeof exesname, spath);

	struct cap_string *cmdline = cap_strnew();
	cap_strapp(cmdline, exesname);
	cap_strapp(cmdline, " ");
	cap_strapp(cmdline, spath);
	cap_strapp(cmdline, " ");
	for (int i = 2; i < argc; ++i) {
		cap_strapp(cmdline, argv[i]);
		cap_strapp(cmdline, " ");
	}

	// Start process communication
	safesystem(cap_strgetc(cmdline));
	
	// Done
	cap_strdel(cmdline);
	return 0;
}
