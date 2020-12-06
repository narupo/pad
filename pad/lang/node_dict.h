#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/cstring.h>
#include <pad/lang/types.h>
#include <pad/lang/gc.h>
#include <pad/lang/nodes.h>

// TODO: test

enum {
    NODE_DICT_ITEM_KEY_SIZE = 256,
};

/**
 * item of array of node_dict_t
 */
typedef struct node_dict_item {
    char key[NODE_DICT_ITEM_KEY_SIZE];  // key of item
    node_t *value;  // value of item
} node_dict_item_t;

/**
 * destruct node_dict_t
 *
 * @param[in] *self pointer to node_dict_t
 */
void
nodedict_del(node_dict_t *self);

/**
 * destruct node_dict_t. do not delete nodes in map
 *
 * @param[in] *self pointer to node_dict_t
 */
void
nodedict_del_without_nodes(node_dict_t *self);

/**
 * destruct node_dict_t with escape array of node_dict_item_t dynamic allocated
 *
 * @param[in] *self pointer to node_dict_t
 *
 * @return success to pointer to array of node_dict_item_t
 * @return failed to NULL
 */
node_dict_item_t *
nodedict_escdel(node_dict_t *self);

/**
 * construct node_dict_t
 *
 * @return success to pointer to node_dict_t (dynamic allocate memory)
 * @return failed to NULL
 */
node_dict_t *
nodedict_new(void);

/**
 * shallow copy node-dict
 *
 * @param[in] *self
 *
 * @return pointer to node_dict_t (copied)
 */
node_dict_t *
nodedict_shallow_copy(node_dict_t *other);

/**
 * deep copy node-dict
 *
 * @param[in] *self
 *
 * @return pointer to node_dict_t (copied)
 */
node_dict_t *
nodedict_deep_copy(const node_dict_t *other);

node_dict_t *
nodedict_resize(node_dict_t *self, int32_t newcapa);

/**
 * move node at key
 *
 * @param[in] *self
 * @param[in] *key        key of strings
 * @param[in] *move_value pointer to node_t (move semantics)
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
node_dict_t *
nodedict_move(node_dict_t *self, const char *key, node_t *move_value);

/**
 * set reference of node at key
 *
 * @param[in] *self
 * @param[in] *key       key of strings
 * @param[in] *ref_value reference to node_t
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
node_dict_t *
nodedict_set(node_dict_t *self, const char *key, node_t *ref_value);

node_dict_item_t *
nodedict_get(node_dict_t *self, const char *key);

const node_dict_item_t *
nodedict_getc(const node_dict_t *self, const char *key);

void
nodedict_clear(node_dict_t *self);

int32_t
nodedict_len(const node_dict_t *self);

const node_dict_item_t *
nodedict_getc_index(const node_dict_t *self, int32_t index);

/**
 * pop node from node dict
 *
 * @param[in] *self
 * @param[in] *key  key of strings
 *
 * @return found to return pointer to node_t
 * @return not found to return NULL
 */
node_t *
nodedict_pop(node_dict_t *self, const char *key);

/**
 * dump node_dict_t at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
nodedict_dump(const node_dict_t *self, FILE *fout);
