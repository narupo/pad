#ifndef WINDOWSPROCESS_H
#define WINDOWSPROCESS_H

#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct WindowsProcess WindowsProcess;

/**
 * @brief 
 *
 * @param self 
*/
void 
winprocess_delete(WindowsProcess* self);

/**
 * @brief 
 *
 * @param void 
 *
 * @return 
*/
WindowsProcess* 
winprocess_new(void);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
bool 
winprocess_start(WindowsProcess* self);

#endif
