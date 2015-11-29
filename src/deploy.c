#include "deploy.h"

static void _Noreturn
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
	exit(EXIT_FAILURE);
}

int
deploy_run(char const* dirname) {
	//! Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	//! Get current path
	char curpath[NFILE_PATH];
	file_solve_path(curpath, sizeof curpath, ".");

	//! Make path
	char dirpath[NFILE_PATH];

	if (!config_path_from_base(config, dirpath, sizeof dirpath, dirname)) {
		WARN("Failed to make path from \"%s\"", dirname);
		goto fail_make_path;
	}

	//! Check path
	if (!file_is_dir(dirpath)) {
		term_eprintf("Not found deploy name \"%s\".\n", dirname);
		goto fail_exists_dir;
	}

	//! Read directory
	DIR* dir = file_opendir(dirpath);
	if (!dir) {
		WARN("Failed to open directory \"%s\"", dirpath);
		goto fail_opendir;
	}
	
	for (;;) {
		//! Read dirent
		errno = 0;
		struct dirent* dirp = readdir(dir);
		if (!dirp) {
			if (errno != 0) {
				WARN("Failed to readdir");
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
		char srcpath[NFILE_PATH];
		char deploypath[NFILE_PATH];

		snprintf(srcpath, sizeof srcpath, "%s/%s", dirpath, name);
		snprintf(deploypath, sizeof deploypath, "%s/%s", curpath, name);

		//! Check exists for override
		if (file_is_exists(deploypath)) {
			term_eprintf("File is exists \"%s\".\n", deploypath);
			continue;
		}

		//! Copy start
		FILE* src = file_open(srcpath, "rb");
		FILE* dst = file_open(deploypath, "wb");

		if (!src || !dst) {
			WARN("Failed to open file");
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
	config_delete(config);
	return 0;

fail_readdir:
	file_closedir(dir);
	config_delete(config);
	return 5;

fail_opendir:
	config_delete(config);
	return 4;

fail_exists_dir:
	config_delete(config);
	return 3;

fail_make_path:
	config_delete(config);
	return 2;

fail_config:
	return 1;
}

int
deploy_main(int argc, char* argv[]) {
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

	again:
		switch (cur) {
			case 0: {
				char const* name = longopts[optsindex].name;
				if (strcmp("help", name) == 0) {
					cur = 'h';
					goto again;
				}
			} break;
			case 'h': {
				deploy_usage();
			} break;
			case '?':
			default: {
				die("Unknown option");
			} break;
		}
	}

	if (argc < optind) {
		die("Failed to parse option");
		return 1;
	} else if (argc < 2) {
		deploy_usage();
	}

	return deploy_run(argv[1]);
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

	return 0;
}
#endif

