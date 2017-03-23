/**
 * Cap.
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2017
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

enum {
    CAP_URL_NHOST = 256,
    CAP_URL_NBUF = 256,
};

/**
 * URL object for parse URL.
 *
 */
struct cap_url;

/**
 * Destruct object.
 *
 * @param[in] self 
 */
void
cap_urldel(struct cap_url *self);

/**
 * Construct object.
 *
 * @param[in] self 
 * @return success to pointer to allocate memory 
 * @return failed to NULL
 */
struct cap_url *
cap_urlnew(void);

/**
 * Parse url string.
 * WARN: Host and port only.
 *
 * @param[in] self 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_url *
cap_urlparse(struct cap_url *self, const char *url);

/**
 * Get host name.
 *
 * @param[in] self 
 * @return success to pointer to host name
 * @return failed to NULL
 */
const char *
cap_urlhost(const struct cap_url *self);

/**
 * Get number of port 
 *
 * @param[in] self 
 * @return success to number of port
 * @return failed to NULL
 */
int32_t
cap_urlport(const struct cap_url *self);
