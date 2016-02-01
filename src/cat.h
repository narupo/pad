#ifndef CAT_H
#define CAT_H

#include "util.h"
#include "file.h"
#include "config.h"
#include "term.h"
#include "string.h"
#include "atcap.h"
#include "strarray.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

/**
 * @brief      Display usage of command
 */
void
cat_usage(void);

/**
 * @brief      Entry function of cat command
 *
 * @param[in]  argc  like a main function's argument
 * @param[in]  argv  like a main function's argument
 *
 * @return     success to a number of zero
 * @return     failed to a number of caperr
 */
int
cat_main(int argc, char* argv[]);

/**
 * This Interface for the cap make command
 * Do not clear and delete CapFile because append row to it for make
 *
 * @param      config   pointer to Config
 * @param      dstfile  pointer to destination CapFile
 * @param[in]  argc     number of elements of argv
 * @param      argv     pointer to array of pointer
 *
 * @return     success to a number of zero
 * @return     failed to a number of caperr
 */
int
cat_make(Config const* config, CapFile* dst, int argc, char* argv[]);

#endif
