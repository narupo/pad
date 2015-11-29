#include "ls.h"

static bool opt_is_all_disp;

void _Noreturn
ls_usage(void) {
	term_eprintf(
		"Usage: cap ls\n"
		"\n"
		"\t-h, --help\tdisplay usage.\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

static int
ls_run(void) {
	// Construct Config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to config new");
		goto fail_1;
	}

	// Open directory from cd path
	char const* cdpath = config_path(config, "cd");
	
	DIR* dir = file_opendir(cdpath);
	if (!dir) {
		WARNF("Failed to opendir \"%s\"", cdpath);
		goto fail_2;
	}

	/// Display file list
	for (;;) {
		// Read dirent
		errno = 0;
		struct dirent* dirp = readdir(dir);
		if (!dirp) {
			if (errno != 0) {
				WARN("Failed to readdir \"%s\"", cdpath);
				goto fail_3;
			} else {
				goto done; 
			}
		}

		// Skip "." and ".."
		if (!opt_is_all_disp) {
			if (strncmp(dirp->d_name, ".", 1) == 0 ||
				strncmp(dirp->d_name, "..", 2) == 0) {
				continue;
			}
		}

		// Display file
		term_printf("%s\n", dirp->d_name);
	}

done:
	closedir(dir);
	config_delete(config);
	return 0;

fail_3:
	closedir(dir);
	config_delete(config);
	return 3;

fail_2:
	config_delete(config);
	return 2;

fail_1:
	return 1;
}

int
ls_main(int argc, char* argv[]) {
	// Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "ha", longopts, &optsindex);
		if (cur == -1)
			break;

	again:
		switch (cur) {
		case 0: {
			char const* name = longopts[optsindex].name;
			if (strcmp("help", name) == 0) {
				cur = 'h';
				goto again;
			}
		} break;
		case 'h': ls_usage(); break;
		case 'a': opt_is_all_disp = !opt_is_all_disp; break;
		case '?':
		default: die("Unknown option"); break;
		}
	}

	if (argc > optind) {
		WARN("Failed to parse option");
		return 1;
	}

	// Run
	return ls_run();
}
