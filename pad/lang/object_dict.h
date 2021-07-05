#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/cstring.h>
#include <pad/lang/types.h>
#include <pad/lang/gc.h>
#include <pad/lang/object.h>

/**
 * constant numbers
 */
enum {
    PAD_OBJ_DICT__ITEM_KEY_SIZE = 256,
};

/**
 * item of array of PadObjDict
 */
typedef struct PadObjDictItem {
    char key[PAD_OBJ_DICT__ITEM_KEY_SIZE];  // key of item
    PadObj *value;  // value of item
} PadObjDictItem;

/**
 * destruct PadObj_dict_t
 *
 * @param[in] *self pointer to PadObjDict
 */
void
PadObjDict_Del(PadObjDict *self);

/**
 * destruct PadObj_dict_t with Pad_Escape array of PadObjDictItem dynamic allocated
 *
 * @param[in] *self pointer to PadObjDict
 *
 * @return success to pointer to array of PadObjDictItem
 * @return failed to NULL
 */
PadObjDictItem *
PadObjDict_EscDel(PadObjDict *self);

/**
 * construct PadObj_dict_t
 *
 * @param[in] *ref_gc reference to PadGc (do not delete)
 *
 * @return success to pointer to PadObjDict (dynamic allocate memory)
 * @return failed to NULL
 */
PadObjDict *
PadObjDict_New(PadGc *ref_gc);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
PadObjDict*
PadObjDict_DeepCopy(const PadObjDict *other);

/**
 * shallow copy
 *
 * @param[in] *other
 *
 * @return pointer to PadObjDict (shallow copied)
 */
PadObjDict *
PadObjDict_ShallowCopy(const PadObjDict *other);

/**
 * resize map
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return
 */
PadObjDict *
PadObjDict_Resize(PadObjDict *self, int32_t newcapa);

/**
 * move object at key
 *
 * @param[in] *self
 * @param[in] *key        key of strings
 * @param[in] *move_value pointer to PadObj (move semantics)
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadObjDict *
PadObjDict_Move(PadObjDict *self, const char *key, PadObj *move_value);

/**
 * set reference of object at key
 *
 * @param[in] *self
 * @param[in] *key       key of strings
 * @param[in] *ref_value reference to PadObj
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadObjDict *
PadObjDict_Set(PadObjDict *self, const char *key, PadObj *ref_value);

/**
 * get dict item
 *
 * @param[in] *self
 * @param[in] *key
 *
 * @return
 */
PadObjDictItem *
PadObjDict_Get(PadObjDict *self, const char *key);

/**
 * get dict item read-only
 *
 * @param[in] *self
 * @param[in] *key
 *
 * @return
 */
const PadObjDictItem *
PadObjDict_Getc(const PadObjDict *self, const char *key);

/**
 * clear state
 * 
 * @param[in] *self 
 */
void
PadObjDict_Clear(PadObjDict *self);

/**
 * get length
 * 
 * @param[in] *self 
 * 
 * @return 
 */
int32_t
PadObjDict_Len(const PadObjDict *self);

/**
 * get dict item by number of index
 * 
 * @param[in] *self 
 * @param[in] index number of index
 * 
 * @return if index is exists then return pointer to item else return NULL
 */
const PadObjDictItem *
PadObjDict_GetcIndex(const PadObjDict *self, int32_t index);

/**
 * get dict item by number of index
 * 
 * @param[in] *self 
 * @param[in] index number of index
 * 
 * @return if index is exists then return pointer to item else return NULL
 */
PadObjDictItem *
PadObjDict_GetIndex(PadObjDict *self, int32_t index);

/**
 * pop object from object dict
 *
 * @param[in] *self
 * @param[in] *key  key of strings
 *
 * @return found to return pointer to PadObj
 * @return not found to return NULL
 */
PadObj *
PadObjDict_Pop(PadObjDict *self, const char *key);

/**
 * dump PadObjDict at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadObjDict_Dump(const PadObjDict *self, FILE *fout);
