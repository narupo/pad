#ifndef LINUXPROCESS_H
#define LINUXPROCESS_H

#include <sys/wait.h>

typedef struct {
	int pipefds[2];
} LinuxProcess;

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
linprocess_start(LinuxProcess* self) {
	switch (fork()) {
	case -1: perror("fork"); break;
	case 0: break;
	default: return true; break;
	}
}

#endif

