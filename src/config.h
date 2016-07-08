#ifndef CAP_CONFPATH_H
#define CAP_CONFPATH_H

#include "string.h"
#include "error.h"
#include "hash.h"
#include "map.h"

#include <stdio.h>
#include <stdlib.h>

struct cap_config;

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cap_confdel(struct cap_config *self);

struct cap_config *
cap_confnew(void);

struct cap_config *
cap_confnewfile(const char *fname);

struct cap_config *
cap_confnewload(void);

/**
 * 
 *
 * @param[in]  *self  
 * @param[in]  *fname 
 *
 * @return 
 */
bool 
cap_confloadfile(struct cap_config *self, const char *fname);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *key  
 *
 * @return 
 */
const char * 
cap_confgetc(const struct cap_config *self, const char *key);

char *
cap_confgetcp(const struct cap_config *self, const char *key);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *fout 
 */
void 
cap_confdump(const struct cap_config *self, FILE *fout);

#endif
