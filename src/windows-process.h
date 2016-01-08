#ifndef WINDOWSPROCESS_H
#define WINDOWSPROCESS_H

#include <windows.h>
#include <stdbool.h>

typedef struct {
	int debug;
} WindowsProcess;

void
winprocess_delete(WindowsProcess* self) {
	if (self) {
		free(self);
	}
}

WindowsProcess*
winprocess_new(void) {
	WindowsProcess* self = (WindowsProcess*) calloc(1, sizeof(WindowsProcess));
	if (!self) {
		perror("WindowsProcess");
		return NULL;
	}

	return self;
}

bool
winprocess_start(WindowsProcess* self) {
	return false;
}

#endif

