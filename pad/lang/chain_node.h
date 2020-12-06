#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/nodes.h>

/**
 * number of type of chain_node_t
 */
typedef enum {
    CHAIN_NODE_TYPE_DOT,
    CHAIN_NODE_TYPE_CALL,
    CHAIN_NODE_TYPE_INDEX,
} chain_node_type_t;

/**
 * destruct chain_node_t
 *
 * @param[in] *self
 */
void
chain_node_del(chain_node_t *self);

/**
 * construct chain_node_t
 *
 * @param[in] type
 * @param[in] *move_factor
 * @param[in] *move_nodearr
 *
 * @return
 */
chain_node_t *
chain_node_new(chain_node_type_t type, node_t *move_noder);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to chain_node_t (copied)
 * @return failed to NULL
 */
chain_node_t *
chain_node_deep_copy(const chain_node_t *other);

/**
 * get type
 *
 * @param[in] *self
 *
 * @return number of type
 */
chain_node_type_t
chain_node_getc_type(const chain_node_t *self);

/**
 * get factor node
 *
 * @param[in] *self
 *
 * @return pointer to node_t
 */
node_t *
chain_node_get_node(chain_node_t *self);

/**
 * get factor node read-only
 *
 * @param[in] *self
 *
 * @return pointer to node_t
 */
const node_t *
chain_node_getc_node(const chain_node_t *self);
