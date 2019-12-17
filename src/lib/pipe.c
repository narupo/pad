#include "lib/pipe.h"

struct pipe {
#ifdef PIPE_WINDOWS
    HANDLE handles[2];
#else
    int fds[2];
#endif
};

void
pipe_del(pipe_t *self) {
    if (!self) {
        return;
    }

    pipe_close(self);
    free(self);
}

pipe_t *
pipe_new(void) {
    pipe_t *self = calloc(1, sizeof(*self));
    if (!self) {
        perror("calloc");
        exit(1);
    }

    return self;
}

pipe_t *
pipe_close(pipe_t *self) {
#ifdef PIPE_WINDOWS
    CloseHandle(self->handles[PIPE_READ]);
    self->handles[PIPE_READ] = NULL;
    CloseHandle(self->handles[PIPE_WRITE]);
    self->handles[PIPE_WRITE] = NULL;
#else
    close(self->fds[PIPE_READ]);
    self->fds[PIPE_READ] = -1;
    close(self->fds[PIPE_WRITE]);
    self->fds[PIPE_WRITE] = -1;
#endif
    return self;
}

pipe_t *
pipe_open(pipe_t *self, int flags) {
    if (!pipe_close(self)) {
        return NULL;
    }

#ifdef PIPE_WINDOWS
    HANDLE hs[2];
    if (!CreatePipe(&hs[PIPE_READ], &hs[PIPE_WRITE], NULL, 0)) {
        return NULL;
    }

    if (!DuplicateHandle(
        GetCurrentProcess(),
        hs[PIPE_READ],
        GetCurrentProcess(),
        &self->handles[PIPE_READ],
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )) {
        return NULL;
    }
    CloseHandle(hs[PIPE_READ]);

    if (!DuplicateHandle(
        GetCurrentProcess(),
        hs[PIPE_WRITE],
        GetCurrentProcess(),
        &self->handles[PIPE_WRITE],
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS
    )) {
        CloseHandle(self->handles[PIPE_READ]);
        return NULL;
    }
    CloseHandle(hs[PIPE_WRITE]);
#else
    if (pipe(self->fds) == -1) {
        return NULL;
    }
#endif
    return self;
}
