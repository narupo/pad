#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <lib/memory.h>
#include <lang/types.h>
#include <lang/object.h>
#include <lang/gc.h>

struct object_array;
typedef struct object_array object_array_t;

/*****************
* delete and new *
*****************/

void
objarr_del(object_array_t* self);

void
objarr_del_without_objs(object_array_t* self);

object_array_t*
objarr_new(void);

object_array_t*
objarr_new_other(object_array_t *other);

/*********
* getter *
*********/

int32_t
objarr_len(const object_array_t *self);

int32_t
objarry_capa(const object_array_t *self);

object_t *
objarr_get(const object_array_t *self, int32_t index);

const object_t *
objarr_getc(const object_array_t *self, int32_t index);

/*********
* setter *
*********/

object_array_t *
objarr_resize(object_array_t* self, int32_t capa);

object_array_t *
objarr_move(object_array_t* self, int32_t index, object_t *move_obj);

/**
 * set referencet at index
 *
 * @param[in] *self
 * @param[in] index    number of index
 * @param[in] *ref_obj reference of object_t
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
object_array_t *
objarr_set(object_array_t* self, int32_t index, object_t *ref_obj);

object_array_t *
objarr_moveb(object_array_t* self, object_t *move_obj);

object_array_t *
objarr_movef(object_array_t* self, object_t *move_obj);

object_array_t *
objarr_pushb(object_array_t* self, object_t *reference);

object_array_t *
objarr_pushf(object_array_t* self, object_t *reference);

object_t *
objarr_popb(object_array_t *self);

object_t *
objarr_get_last(object_array_t *self);

const object_t *
objarr_getc_last(const object_array_t *self);

/**
 * dump object array at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
objarr_dump(const object_array_t *self, FILE *fout);