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

struct cap_url;

void
cap_urldel(struct cap_url *self);

struct cap_url *
cap_urlnew(void);

/**
 * Host and port only.
 */
struct cap_url *
cap_urlparse(struct cap_url *self, const char *url);

const char *
cap_urlhost(const struct cap_url *self);

int32_t
cap_urlport(const struct cap_url *self);
