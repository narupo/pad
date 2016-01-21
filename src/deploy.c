#include "deploy.h"

static char const* PROGNAME = "cap deploy";

void
deploy_usage(void) {
	term_eprintf(
		"cap deploy\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap deploy [deploy-name] [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help\tdisplay usage\n"
		"\n"
	);
}

int
deploy_run(char const* dirname) {
	// Load config
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	// Get current path
	char curpath[FILE_NPATH];
	if (!file_solve_path(curpath, sizeof curpath, ".")) {
		return caperr(PROGNAME, CAPERR_ERROR, "Failed to solve path \".\"");
	}

	// Make path
	char dirpath[FILE_NPATH];

	if (!config_path_with_cd(config, dirpath, sizeof dirpath, dirname)) {
		return caperr(PROGNAME, CAPERR_ERROR, "Failed to make path from \"%s\"", dirname);
	}

	// Check path
	if (!file_is_dir(dirpath)) {
		return caperr(PROGNAME, CAPERR_NOTFOUND, "deploy name \"%s\".\n", dirname);
	}

	// Read directory
	Directory* dir = dir_open(dirpath);
	if (!dir) {
		return caperr(PROGNAME, CAPERR_OPENDIR, "\"%s\"", dirpath);
	}
	
	for (DirectoryNode* dirnode; (dirnode = dir_read_node(dir)); dirnode_delete(dirnode)) {
		char const* nodename = dirnode_name(dirnode);

		// Skip "." and ".."
		if (strncmp(nodename, ".", 1) == 0 || strncmp(nodename, "..", 2) == 0) {
			continue;
		}

		// Deploy files to current directory

		// Make deploy file path
		char srcpath[FILE_NPATH];
		char deploypath[FILE_NPATH];

		snprintf(srcpath, sizeof srcpath, "%s/%s", dirpath, nodename);
		snprintf(deploypath, sizeof deploypath, "%s/%s", curpath, nodename);

		// Check exists for override
		if (file_is_exists(deploypath)) {
			caperr(PROGNAME, CAPERR_ERROR, "File is exists \"%s\".\n", deploypath);
			continue;
		}

		// Copy start
		FILE* src = file_open(srcpath, "rb");
		FILE* dst = file_open(deploypath, "wb");

		if (!src || !dst) {
			caperr(PROGNAME, CAPERR_FOPEN, "");
			file_close(src);
			file_close(dst);
			continue;
		}

		int ch;
		while ((ch = fgetc(src)) != EOF) {
			fputc(ch, dst);
		}
		file_close(src);
		file_close(dst);
	}

	// Done
	dir_close(dir);
	return 0;
}

int
deploy_main(int argc, char* argv[]) {
	// Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
			case 'h': {
				deploy_usage();
				return 0;
			} break;
			case '?':
			default: {
				return caperr(PROGNAME, CAPERR_INVALID, "option");
			} break;
		}
	}

	if (argc < optind) {
		return caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
	} else if (argc < 2) {
		deploy_usage();
		return 0;
	}

	return deploy_run(argv[1]);
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

	return 0;
}
#endif

