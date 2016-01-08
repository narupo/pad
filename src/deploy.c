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
	//! Load config
	Config* config = config_instance();
	if (!config) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
		goto fail_config;
	}

	//! Get current path
	char curpath[FILE_NPATH];
	if (!file_solve_path(curpath, sizeof curpath, ".")) {
		caperr(PROGNAME, CAPERR_ERROR, "Failed to solve path \".\"");
		goto fail_solve_path;
	}

	//! Make path
	char dirpath[FILE_NPATH];

	if (!config_path_from_base(config, dirpath, sizeof dirpath, dirname)) {
		caperr(PROGNAME, CAPERR_ERROR, "Failed to make path from \"%s\"", dirname);
		goto fail_make_path;
	}

	//! Check path
	if (!file_is_dir(dirpath)) {
		caperr(PROGNAME, CAPERR_NOTFOUND, "deploy name \"%s\".\n", dirname);
		goto fail_exists_dir;
	}

	//! Read directory
	DIR* dir = file_opendir(dirpath);
	if (!dir) {
		caperr(PROGNAME, CAPERR_OPENDIR, "\"%s\"", dirpath);
		goto fail_opendir;
	}
	
	for (;;) {
		//! Read dirent
		errno = 0;
		struct dirent* dirp = readdir(dir);
		if (!dirp) {
			if (errno != 0) {
				caperr(PROGNAME, CAPERR_READDIR, "");
				goto fail_readdir;
			} else {
				//! End of readdir
				goto done;
			}
		}

		char const* name = dirp->d_name;

		//! Skip "." and ".."
		if (strncmp(name, ".", 1) == 0 ||
			strncmp(name, "..", 2) == 0) {
			continue;
		}

		//! Deploy files to current directory

		//! Make deploy file path
		char srcpath[FILE_NPATH];
		char deploypath[FILE_NPATH];

		snprintf(srcpath, sizeof srcpath, "%s/%s", dirpath, name);
		snprintf(deploypath, sizeof deploypath, "%s/%s", curpath, name);

		//! Check exists for override
		if (file_is_exists(deploypath)) {
			caperr(PROGNAME, CAPERR_ERROR, "File is exists \"%s\".\n", deploypath);
			continue;
		}

		//! Copy start
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

done:
	file_closedir(dir);
	return 0;

fail_readdir:
	file_closedir(dir);
	return 6;

fail_opendir:
	return 5;

fail_exists_dir:
	return 4;

fail_make_path:
	return 3;

fail_solve_path:
	return 2;

fail_config:
	return 1;
}

int
deploy_main(int argc, char* argv[]) {
	//! Parse options
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

