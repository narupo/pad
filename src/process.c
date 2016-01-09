#include "process.h"

typedef enum {
	ProcessTypeLinux,
	ProcessTypeWindows,
} ProcessType;

struct Process {
	ProcessType type;
#if defined(PROCESS_WINDOWS)
	WindowProcess* process;
#elif defined(PROCESS_LINUX)
	LinuxProcess* process;
#endif
};

void
process_delete(Process* self) {
	if (self) {
#if defined(PROCESS_WINDOWS)
		winprocess_delete(self->process);
#elif defined(PROCESS_LINUX)
		linprocess_delete(self->process);
#endif
		free(self);
	}
}

Process*
process_new(void) {
	Process* self = (Process*) calloc(1, sizeof(Process));
	if (!self) {
		perror("Process");
		return NULL;
	}

#if defined(PROCESS_WINDOWS)
	self->type = ProcessTypeWindows;
	self->process = winprocess_new();
	if (!self->process) {
		free(self);
		perror("WindowProcess");
		return NULL;
	}
#elif defined(PROCESS_LINUX)
	self->type = ProcessTypeLinux;
	self->process = linprocess_new();
	if (!self->process) {
		free(self);
		perror("LinuxProcess");
		return NULL;
	}
#endif

	return self;
}

/*****************
* Process getter *
*****************/

char const*
process_type_to_string(Process const* self) {
	switch (self->type) {
	default: return "Unknown type of process"; break;
	case ProcessTypeLinux: return "Linux"; break;
	case ProcessTypeWindows: return "Windows"; break;
	}
}

/******************
* Process wrapper *
******************/

bool
process_start(Process* self, char const* cmdname) {
#if defined(PROCESS_WINDOWS)
	return winprocess_start(self->process);
#elif defined(PROCESS_LINUX)
	return linprocess_start(self->process, cmdname);
#endif
}

#if defined(TEST_PROCESS)
#include <stdio.h>

int
test_win(int argc, char* argv[]) {
#if defined(PROCESS_WINDOWS)
	LPSTR cmdline = "notepad.exe";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));
	
	if (CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("running...\n");

		// Wait
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Get exit status of child process
		DWORD res;
		GetExitCodeProcess(pi.hProcess, &res);

		// Done
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		printf("exit code=%ld\n", res);
	}
#endif
	return 0;
}

int
test_process(int argc, char* argv[]) {
	Process* proc = process_new();

	// fprintf(stderr, "Process type \"%s\"\n", process_type_to_string(proc));
	process_start(proc, "ls");

	process_delete(proc);
	return 0;
}

int
main(int argc, char* argv[]) {
	//return test_win(argc, argv);
	return test_process(argc, argv);
}

#endif

