#pragma once

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
# define PIPE_WINDOWS 1
# include <windows.h>
#else
# include <unistd.h>
#endif

enum {
    PIPE_READ = 0,
    PIPE_WRITE = 1,
};

struct pipe;
typedef struct pipe pipe_t;

/**
 * Destruct PadObj
 *
 * @param[in] *self 
 */
void 
pipe_del(pipe_t *self);

/**
 * Construct PadObj
 * If failed to construct then show errors and exit from process
 *
 * @param[in] void 
 *
 * @return success to pointer to pipe_t dynamic allocate memory
 */
pipe_t * 
pipe_new(void);

/**
 * Close pipe
 *
 * @param[in] *self 
 *
 * @return success to pointer to self
 * @return faield to pointer to NULL
 */
pipe_t * 
pipe_close(pipe_t *self);

/**
 * Open pipe
 *
 * @param[in] *self 
 * @param[in] flags number of open flags
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
pipe_t * 
pipe_open(pipe_t *self, int flags);
