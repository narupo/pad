/**
 * Cap
 *
 * B class
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef MAP_H
#define MAP_H

#define _GNU_SOURCE 1 /* cap: map.h: strdup */
#include <stdbool.h>
#include <ctype.h>

#include "error.h"
#include "string.h"
#include "hash.h"

/**********
* cap_map *
**********/

struct cap_map;

/**
 * Destruct map
 *
 * @param[in] *self 
 */
void 
cap_mapdel(struct cap_map *self);

/**
 * Construct map.
 * 
 * @return success to pointer to dynamic allocate memory
 * @return failed to NULL
 */
struct cap_map *
cap_mapnew(void);

/**
 * Set value at key element.
 * 
 * @param[in] *self 
 * @param[in] *key  string of key
 * @param[in] *val  string of value
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_map *
cap_mapset(struct cap_map *self, const char *key, const char *val);

/**
 * Get value by key.
 *
 * @param[in]  *self 
 * @param[in]  *key  string of key
 *
 * @return found to pointer to value
 * @return not found to NULL
 */
const char * 
cap_mapgetc(struct cap_map *self, const char *key);

/**
 * Get copy value by key.
 *
 * @param[in]  *self 
 * @param[in]  *key  string of key
 *
 * @return found to pointer to copy value with dynamic allocate memory
 * @return not found or failed to NULL
 */
char * 
cap_mapgetcp(struct cap_map *self, const char *key);

#endif
