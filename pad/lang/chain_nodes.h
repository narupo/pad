#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/chain_node.h>

/**
 * destruct chain_nodes_t
 *
 * @param[in] *self
 */
void
chain_nodes_del(chain_nodes_t *self);

/**
 * construct chain_nodes_t
 *
 * @param[in] void
 *
 * @return pointer to chain_nodes_t
 */
chain_nodes_t *
chain_nodes_new(void);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to chain_nodes_t (copied)
 * @return failed to NULL
 */
chain_nodes_t *
chain_nodes_deep_copy(const chain_nodes_t *other);

/**
 * resize chain_nodes_t
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
chain_nodes_t *
chain_nodes_resize(chain_nodes_t *self, int32_t newcapa);

/**
 * move back pointer to chain_node_t
 *
 * @param[in] *self
 * @param[in] *move_chain_node
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
chain_nodes_t *
chain_nodes_moveb(chain_nodes_t *self, chain_node_t *move_chain_node);

/**
 * get length of array
 *
 * @param[in] *self
 *
 * @return number of length
 */
int32_t
chain_nodes_len(const chain_nodes_t *self);

/**
 * get chain_node_t from array
 *
 * @param[in] *self
 * @param[in] idx
 *
 * @return success to pointer to chain_node_t
 * @return failed to NULL
 */
chain_node_t *
chain_nodes_get(chain_nodes_t *self, int32_t idx);
