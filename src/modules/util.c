/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "util.h"

void
freeargv(int argc, char *argv[]) {
	if (argv) {
		for (int i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
	}
}

void
showargv(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		printf("%s\n", argv[i]);
	}
}

bool
isoutofhome(const char *varhome, const char *pth) {
	char hm[FILE_NPATH];
	if (!file_readline(hm, sizeof hm, varhome)) {
		err_error("invalid environment variable of varhome");
		return true;
	}

	char home[FILE_NPATH];
	char path[FILE_NPATH];

	if (!file_solve(home, sizeof home, hm) ||
		!file_solve(path, sizeof path, pth)) {
		err_error("failed to solve path");
		return true;
	}

	if (!file_exists(path)) {
		return true;
	}

	size_t homelen = strlen(home);
	if (strncmp(home, path, homelen)) {
		return true;
	}

	return false;
}

int
randrange(int min, int max) {
	return min + (int)(rand() * (max - min + 1.0) / (1.0 + RAND_MAX));
}

int
safesystem(const char *cmdline) {
#ifdef _CAP_WINDOWS
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        (char *)cmdline,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)            // Pointer to PROCESS_INFORMATION structure
    ) {
        err_error("failed to create sub process");
        return 1;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
	cl_t *cl = cl_new();
	if (!cl_parse_str_opts(cl, cmdline, 0)) {
		err_error("failed to parse command line \"%s\"", cmdline);
		cl_del(cl);
		return -1;
	}

	int argc = cl_len(cl);
	char **argv = cl_escdel(cl);
	if (!argv) {
		err_error("failed to escape and delete of clk");
		return -1;
	}
	
	switch (fork()) {
	case -1:
		err_error("failed to fork");
		return -1;
	break;
	case 0:
		if (execv(argv[0], argv) == -1) {
			err_error("failed to safesystem");
			freeargv(argc, argv);
			_exit(1);
		}
	break;
	default:
		freeargv(argc, argv);
		wait(NULL);
	break;
	}
#endif
	return 0;
}

/*
struct cap_array *
argsbyoptind(int argc, char *argv[], int optind) {
	struct cap_array *args = cap_arrnew();
	
	// DO NOT DELETE FOR DEBUG.
	//
	// printf("argc[%d] optind[%d]\n", argc, optind);
	// for (int i = 0; i < argc; ++i) {
	// 	printf("%d %s\n", i, argv[i]);
	// }
	

	cap_arrpush(args, argv[0]);
	for (int i = optind; i < argc; ++i) {
		cap_arrpush(args, argv[i]);
	}

	return args;
}
*/
