#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/nodes.h>

/**
 * number of type of PadChainNode
 */
typedef enum {
    PAD_CHAIN_NODE_TYPE___DOT,
    PAD_CHAIN_NODE_TYPE___CALL,
    PAD_CHAIN_NODE_TYPE___INDEX,
} PadChainNodeType;

/**
 * destruct PadChainNode
 *
 * @param[in] *self
 */
void
PadChainNode_Del(PadChainNode *self);

/**
 * construct PadChainNode
 *
 * @param[in] type
 * @param[in] *move_factor
 * @param[in] *move_nodearr
 *
 * @return
 */
PadChainNode *
PadChainNode_New(PadChainNodeType type, PadNode *move_noder);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to PadChainNode (copied)
 * @return failed to NULL
 */
PadChainNode *
PadChainNode_DeepCopy(const PadChainNode *other);

/**
 * get type
 *
 * @param[in] *self
 *
 * @return number of type
 */
PadChainNodeType
PadChainNode_GetcType(const PadChainNode *self);

/**
 * get factor node
 *
 * @param[in] *self
 *
 * @return pointer to PadNode
 */
PadNode *
PadChainNode_GetNode(PadChainNode *self);

/**
 * get factor node read-only
 *
 * @param[in] *self
 *
 * @return pointer to PadNode
 */
const PadNode *
PadChainNode_GetcNode(const PadChainNode *self);
