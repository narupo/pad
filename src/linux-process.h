#ifndef LINUXPROCESS_H
#define LINUXPROCESS_H

#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct LinuxProcess LinuxProcess;

/**
 * @brief 
 *
 * @param self 
*/
void 
linprocess_delete(LinuxProcess* self);

/**
 * @brief 
 *
 * @param void 
 *
 * @return 
*/
LinuxProcess* 
linprocess_new(void);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
bool 
linprocess_start(LinuxProcess* self, char const* cmdline);

#endif
