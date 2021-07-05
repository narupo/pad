#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/chain_node.h>

/**
 * destruct PadChainNodes
 *
 * @param[in] *self
 */
void
PadChainNodes_Del(PadChainNodes *self);

/**
 * construct PadChainNodes
 *
 * @param[in] void
 *
 * @return pointer to PadChainNodes
 */
PadChainNodes *
PadChainNodes_New(void);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to PadChainNodes (copied)
 * @return failed to NULL
 */
PadChainNodes *
PadChainNodes_DeepCopy(const PadChainNodes *other);

/**
 * resize PadChainNodes
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadChainNodes *
PadChainNodes_Resize(PadChainNodes *self, int32_t newcapa);

/**
 * move back pointer to PadChainNode
 *
 * @param[in] *self
 * @param[in] *move_chain_node
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadChainNodes *
PadChainNodes_MoveBack(PadChainNodes *self, PadChainNode *move_chain_node);

/**
 * get length of array
 *
 * @param[in] *self
 *
 * @return number of length
 */
int32_t
PadChainNodes_Len(const PadChainNodes *self);

/**
 * get PadChainNode from array
 *
 * @param[in] *self
 * @param[in] idx
 *
 * @return success to pointer to PadChainNode
 * @return failed to NULL
 */
PadChainNode *
PadChainNodes_Get(PadChainNodes *self, int32_t idx);
