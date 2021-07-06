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
    PAD_PIPE__READ = 0,
    PAD_PIPE__WRITE = 1,
};

struct PadPipe;
typedef struct PadPipe PadPipe;

/**
 * Destruct PadObj
 *
 * @param[in] *self 
 */
void 
PadPipe_Del(PadPipe *self);

/**
 * Construct PadObj
 * If failed to construct then show errors and exit from process
 *
 * @param[in] void 
 *
 * @return success to pointer to PadPipe dynamic allocate memory
 */
PadPipe * 
PadPipe_New(void);

/**
 * Close pipe
 *
 * @param[in] *self 
 *
 * @return success to pointer to self
 * @return faield to pointer to NULL
 */
PadPipe * 
PadPipe_Close(PadPipe *self);

/**
 * Open pipe
 *
 * @param[in] *self 
 * @param[in] flags number of open flags
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
PadPipe * 
PadPipe_Open(PadPipe *self, int flags);
