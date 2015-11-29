#include "edit.h"

enum {
	NCOMMAND = 256,
};

static void _Noreturn
edit_usage(void) {
	term_eprintf(
		"cap edit\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap edit file-name [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help\tdisplay usage\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

int
edit_main(int argc, char* argv[]) {
	char const* editpath = "/usr/bin/vi";
	char const* fname = NULL;
	
	//! Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 0: {
			char const* name = longopts[optsindex].name;
			if (strcmp("help", name) == 0) {
				edit_usage();
			}
		} break;
		case 'h':
			edit_usage();
			break;
		case '?':
		default:
			die("Unknown option");
			break;
		}
	}

	if (argc < optind) {
		die("Failed to parse option");
		goto fail_parse_option;
	}

	//! Get edit file base name
	if (argc == optind) {
		edit_usage();
	}
	fname = argv[optind];

	//! Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	//! Make file path from edit file base name
	char spath[NFILE_PATH];
	if (!config_path_from_base(config, spath, sizeof spath, fname)) {
		WARN("Failed to make path from \"%s\"", fname);
		goto fail_make_path;
	}

	//! Execute command with fork, exec family, wait
	pid_t pid = fork();

	if (pid < 0) {
		WARN("Failed to fork");
		goto fail_fork;
	} else if (pid == 0) {
		//! Are child process
		execl(editpath, editpath, spath, NULL); //< Good bye!
		WARN("Failed to execl");
		goto fail_exec;
	}

	//! Are parent process
	config_delete(config);

	if (wait(NULL) == -1) {
		WARN("Failed to wait");
	}
	return 0;

fail_exec:
	config_delete(config);
	return 5;

fail_fork:
	config_delete(config);
	return 4;

fail_make_path:
	config_delete(config);
	return 3;

fail_config:
	return 2;

fail_parse_option:
	return 1;
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

	return 0;
}
#endif

