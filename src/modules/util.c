/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
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

bool
is_out_of_home(const char *homepath, const char *argpath) {
    char home[FILE_NPATH];
    char path[FILE_NPATH];

    if (!file_solve(home, sizeof home, homepath) ||
        !file_solve(path, sizeof path, argpath)) {
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
safesystem(const char *cmdline, int option) {
#ifdef _CAP_WINDOWS
    int flag = 0;
    if (option & SAFESYSTEM_EDIT) {
        // option for edit command
        flag = CREATE_NEW_CONSOLE;
    } else {
        flag = 0;
    }

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = { sizeof(STARTUPINFO) };

    // Start the child process.
    if (!CreateProcess(NULL, // No module name (use command line)
        (char *) cmdline, // Command line
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        flag, // No creation flags
        NULL, // Use parent's environment block
        NULL, // Use parent's starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi) // Pointer to PROCESS_INFORMATION structure
    ) {
        err_error("failed to create sub process");
        return 1;
    }

    if (option & SAFESYSTEM_EDIT) {
        // case of edit command, to not wait exit of child process
        return 0;
    }

    if (option & SAFESYSTEM_DETACH) {
        // not wait child process
        return 0;
    }

    // success to fork
    HANDLE child_process = pi.hProcess;
    if (!CloseHandle(pi.hThread)) {
        err_error("failed to close handle");
        return 1;
    }

    // wait for child process
    DWORD r = WaitForSingleObject(child_process, INFINITE);
    switch(r) {
    case WAIT_FAILED:
        err_error("child process was failed");
        return 1;
    case WAIT_ABANDONED:
        err_error("child process was abandoned");
        return 1;
    case WAIT_OBJECT_0: // success
        break;
    case WAIT_TIMEOUT:
        err_error("child process was timeout");
        return 1;
    default:
        return 1;
    }

    // get exit code of child process
    DWORD exit_code;
    if (!GetExitCodeProcess(child_process, &exit_code)) {
        err_error("failed to get exit code of child process");
        return 1;
    }

    return exit_code;

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

    return 0;
#endif
}

cstring_array_t *
argsbyoptind(int argc, char *argv[], int optind) {
	cstring_array_t *args = cstrarr_new();

	// DO NOT DELETE FOR DEBUG.
	//
	// printf("argc[%d] optind[%d]\n", argc, optind);
	// for (int i = 0; i < argc; ++i) {
	// 	printf("%d %s\n", i, argv[i]);
	// }

	cstrarr_push(args, argv[0]);
	for (int i = optind; i < argc; ++i) {
		cstrarr_push(args, argv[i]);
	}

	return args;
}

