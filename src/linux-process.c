#include "linux-process.h"

struct LinuxProcess {
	int pipefds[2];
};

void
linprocess_delete(LinuxProcess* self) {
	if (self) {
		free(self);
	}
}

LinuxProcess*
linprocess_new(void) {
	LinuxProcess* self = (LinuxProcess*) calloc(1, sizeof(LinuxProcess));
	if (!self) {
		perror("LinuxProcess");
		return NULL;
	}

	return self;
}

bool
linprocess_start(LinuxProcess* self, char const* cmdline) {
	switch (fork()) {
	case -1:
		perror("fork");
		return false;
		break;

	case 0: // Child process
		break;

	default: // Parent process
		return true;
		break;
	}

	return false;
}

#if defined(TEST_LINUXPROCESS)
int
test_process(int argc, char* argv[]) {
	LinuxProcess* proc = linprocess_new();
	linprocess_start(proc, "ping -t 192.168.11.1");
	linprocess_delete(proc);
	return 0;
}

int
main(int argc, char* argv[]) {
    return test_process(argc, argv);
}
#endif
