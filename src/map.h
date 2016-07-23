#ifndef MAP_H
#define MAP_H

#define _GNU_SOURCE 1 /* In cap: map.h for the strdup */
#include <stdbool.h>
#include <ctype.h>

#include "error.h"
#include "string.h"
#include "hash.h"

struct cap_map;

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cap_mapdel(struct cap_map *self);

struct cap_map *
cap_mapnew(void);

struct cap_map *
cap_mapset(struct cap_map *self, const char *key, const char *val);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *key  
 *
 * @return 
 */
const char * 
cap_mapgetc(struct cap_map *self, const char *key);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *key  
 *
 * @return 
 */
char * 
cap_mapgetcp(struct cap_map *self, const char *key);

#endif
