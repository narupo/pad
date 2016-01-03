#ifndef BRIEF_H
#define BRIEF_H

#include "term.h"
#include "util.h"
#include "config.h"
#include "file.h"
#include "caperr.h"
#include "cap-file.h"
#include "atcap.h"
#include "strarray.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

/**
 * @brief 
 *
 * @param void 
*/
void 
brief_usage(void);

/**
 * @brief 
 *
 * @param argc   
 * @param argv[] 
 *
 * @return 
*/
int 
brief_main(int argc, char* argv[]);

#endif
