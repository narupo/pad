#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>

/**
 * number of type of PadChainObj
 */
typedef enum {
    PAD_CHAIN_PAD_OBJ_TYPE___DOT,
    PAD_CHAIN_PAD_OBJ_TYPE___CALL,
    PAD_CHAIN_PAD_OBJ_TYPE___INDEX,
} PadChainObjType;

/**
 * destruct PadChainObj
 *
 * @param[in] *self
 */
void
PadChainObj_Del(PadChainObj *self);

/**
 * construct PadChainObj
 *
 * @param[in] type
 * @param[in] *move_factor
 * @param[in] *move_objarr
 *
 * @return
 */
PadChainObj *
PadChainObj_New(PadChainObjType type, PadObj *move_obj);

/**
 * TODO: test
 *
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
PadChainObj *
PadChainObj_DeepCopy(const PadChainObj *other);

/**
 * TODO: test
 */
PadChainObj *
PadChainObj_ShallowCopy(const PadChainObj *other);

/**
 * get type
 *
 * @param[in] *self
 *
 * @return number of type
 */
PadChainObjType
PadChainObj_GetcType(const PadChainObj *self);

/**
 * get factor obj
 *
 * @param[in] *self
 *
 * @return pointer to PadObj
 */
PadObj *
PadChainObj_GetObj(PadChainObj *self);

/**
 * get factor obj read-only
 *
 * @param[in] *self
 *
 * @return pointer to PadObj
 */
const PadObj *
PadChainObj_GetcObj(const PadChainObj *self);

/**
 * dump PadChainObj
 *
 * @param[in] *self
 * @param[in] *fout output stream
 */
void
PadChainObj_Dump(const PadChainObj *self, FILE *fout);
