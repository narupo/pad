#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>

/**
 * destruct PadRingObjs
 *
 * @param[in] *self
 */
void
PadChainObjs_Del(PadChainObjs *self);

/**
 * construct PadRingObjs
 *
 * @param[in] void
 *
 * @return pointer to PadChainObjs
 */
PadChainObjs *
PadChainObjs_New(void);

/**
 * TODO: test
 *
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
PadChainObjs *
PadChainObjs_DeepCopy(const PadChainObjs *other);

/**
 * TODO: test
 */
PadChainObjs *
PadChainObjs_ShallowCopy(const PadChainObjs *other);

/**
 * resize PadChainObjs
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadChainObjs *
PadChainObjs_Resize(PadChainObjs *self, int32_t newcapa);

/**
 * move back pointer to PadChainObj
 *
 * @param[in] *self
 * @param[in] *move_chain_obj
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadChainObjs *
PadChainObjs_MoveBack(PadChainObjs *self, PadChainObj *move_chain_obj);

/**
 * get length of array
 *
 * @param[in] *self
 *
 * @return number of length
 */
int32_t
PadChainObjs_Len(const PadChainObjs *self);

/**
 * get PadChainObj from array
 *
 * @param[in] *self
 * @param[in] idx
 *
 * @return success to pointer to PadChainObj
 * @return failed to NULL
 */
PadChainObj *
PadChainObjs_Get(PadChainObjs *self, int32_t idx);

PadChainObj *
PadChainObjs_GetLast(PadChainObjs *self);

PadChainObj *
PadChainObjs_GetLast2(PadChainObjs *self);

/**
 * dump PadChainObjs
 *
 * @param[in] *self
 * @param[in] *fout output stream
 */
void
PadChainObjs_Dump(const PadChainObjs *self, FILE *fout);
