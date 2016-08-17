/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "string.h"
#include "error.h"
#include "hash.h"
#include "map.h"

#include <stdio.h>
#include <stdlib.h>

struct cap_config;

void 
cap_confdel(struct cap_config *self);

struct cap_config *
cap_confnew(void);

struct cap_config *
cap_confnewfile(const char *fname);

struct cap_config *
cap_confnewload(void);

bool 
cap_confloadfile(struct cap_config *self, const char *fname);

const char * 
cap_confgetc(const struct cap_config *self, const char *key);

char *
cap_confgetcp(const struct cap_config *self, const char *key);

void 
cap_confdump(const struct cap_config *self, FILE *fout);

#endif
