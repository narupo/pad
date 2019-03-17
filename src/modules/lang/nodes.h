#pragma once

#include "lib/memory.h"

typedef enum {
    NODE_TYPE_BIN = 1,
} node_type_t;

struct node;
typedef struct node node_t;

/**
 * Destruct node
 *
 * @param[in] self pointer to dynamic allocate memory of node_t
 */
void
node_del(node_t *self);

/**
 * Construct node
 *
 * @return pointer to dynamic allocate memory of node_t
 */
node_t *
node_new(node_type_t type);

