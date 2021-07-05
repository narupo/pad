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
    PAD_NODE_DICT__ITEM_KEY_SIZE = 256,
};

/**
 * item of array of PadNodeDict
 */
typedef struct PadNode_dict_item {
    char key[PAD_NODE_DICT__ITEM_KEY_SIZE];  // key of item
    PadNode *value;  // value of item
} PadNodeDictItem;

/**
 * destruct PadNode_dict_t
 *
 * @param[in] *self pointer to PadNodeDict
 */
void
PadNodeDict_Del(PadNodeDict *self);

/**
 * destruct PadNode_dict_t. do not delete nodes in map
 *
 * @param[in] *self pointer to PadNodeDict
 */
void
PadNodeDict_DelWithoutNodes(PadNodeDict *self);

/**
 * destruct PadNode_dict_t with Pad_Escape array of PadNodeDictItem dynamic allocated
 *
 * @param[in] *self pointer to PadNodeDict
 *
 * @return success to pointer to array of PadNodeDictItem
 * @return failed to NULL
 */
PadNodeDictItem *
PadNodeDict_EscDel(PadNodeDict *self);

/**
 * construct PadNode_dict_t
 *
 * @return success to pointer to PadNodeDict (dynamic allocate memory)
 * @return failed to NULL
 */
PadNodeDict *
PadNodeDict_New(void);

/**
 * shallow copy node-dict
 *
 * @param[in] *self
 *
 * @return pointer to PadNodeDict (copied)
 */
PadNodeDict *
PadNodeDict_ShallowCopy(const PadNodeDict *other);

/**
 * deep copy node-dict
 *
 * @param[in] *self
 *
 * @return pointer to PadNodeDict (copied)
 */
PadNodeDict *
PadNodeDict_DeepCopy(const PadNodeDict *other);

PadNodeDict *
PadNodeDict_Resize(PadNodeDict *self, int32_t newcapa);

/**
 * move node at key
 *
 * @param[in] *self
 * @param[in] *key        key of strings
 * @param[in] *move_value pointer to PadNode (move semantics)
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadNodeDict *
PadNodeDict_Move(PadNodeDict *self, const char *key, PadNode *move_value);

/**
 * set reference of node at key
 *
 * @param[in] *self
 * @param[in] *key       key of strings
 * @param[in] *ref_value reference to PadNode
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadNodeDict *
PadNodeDict_Set(PadNodeDict *self, const char *key, PadNode *ref_value);

PadNodeDictItem *
PadNodeDict_Get(PadNodeDict *self, const char *key);

const PadNodeDictItem *
PadNodeDict_Getc(const PadNodeDict *self, const char *key);

void
PadNodeDict_Clear(PadNodeDict *self);

int32_t
PadNodeDict_Len(const PadNodeDict *self);

const PadNodeDictItem *
PadNodeDict_GetcIndex(const PadNodeDict *self, int32_t index);

/**
 * pop node from node dict
 *
 * @param[in] *self
 * @param[in] *key  key of strings
 *
 * @return found to return pointer to PadNode
 * @return not found to return NULL
 */
PadNode *
PadNodeDict_Pop(PadNodeDict *self, const char *key);

/**
 * dump PadNodeDict at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadNodeDict_Dump(const PadNodeDict *self, FILE *fout);
