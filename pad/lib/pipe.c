#include <pad/lib/pipe.h>

struct PadPipe {
#ifdef PIPE_WINDOWS
    HANDLE handles[2];
#else
    int fds[2];
#endif
};

void
PadPipe_Del(PadPipe *self) {
    if (!self) {
        return;
    }

    PadPipe_Close(self);
    free(self);
}

PadPipe *
PadPipe_New(void) {
    PadPipe *self = calloc(1, sizeof(*self));
    if (!self) {
        perror("calloc");
        exit(1);
    }

    return self;
}

PadPipe *
PadPipe_Close(PadPipe *self) {
#ifdef PIPE_WINDOWS
    CloseHandle(self->handles[PAD_PIPE__READ]);
    self->handles[PAD_PIPE__READ] = NULL;
    CloseHandle(self->handles[PAD_PIPE__WRITE]);
    self->handles[PAD_PIPE__WRITE] = NULL;
#else
    close(self->fds[PAD_PIPE__READ]);
    self->fds[PAD_PIPE__READ] = -1;
    close(self->fds[PAD_PIPE__WRITE]);
    self->fds[PAD_PIPE__WRITE] = -1;
#endif
    return self;
}

PadPipe *
PadPipe_Open(PadPipe *self, int flags) {
    if (!PadPipe_Close(self)) {
        return NULL;
    }

#ifdef PIPE_WINDOWS
    HANDLE hs[2];
    if (!CreatePipe(&hs[PAD_PIPE__READ], &hs[PAD_PIPE__WRITE], NULL, 0)) {
        return NULL;
    }

    if (!DuplicateHandle(
        GetCurrentProcess(),
        hs[PAD_PIPE__READ],
        GetCurrentProcess(),
        &self->handles[PAD_PIPE__READ],
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )) {
        return NULL;
    }
    CloseHandle(hs[PAD_PIPE__READ]);

    if (!DuplicateHandle(
        GetCurrentProcess(),
        hs[PAD_PIPE__WRITE],
        GetCurrentProcess(),
        &self->handles[PAD_PIPE__WRITE],
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )) {
        CloseHandle(self->handles[PAD_PIPE__READ]);
        return NULL;
    }
    CloseHandle(hs[PAD_PIPE__WRITE]);
#else
    if (pipe(self->fds) == -1) {
        return NULL;
    }
#endif
    return self;
}
