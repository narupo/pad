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

object_array_t*
objarr_new(void);

object_array_t*
objarr_new_other(object_array_t *other);

/*********
* getter *
*********/

size_t
objarr_len(const object_array_t *self);

size_t
objarry_capa(const object_array_t *self);

object_t *
objarr_get(const object_array_t *self, size_t index);

const object_t *
objarr_getc(const object_array_t *self, size_t index);

/*********
* setter *
*********/

object_array_t *
objarr_resize(object_array_t* self, size_t capa);

object_array_t *
objarr_move(object_array_t* self, int32_t index, object_t *move_obj);

object_array_t *
objarr_moveb(object_array_t* self, object_t *obj);

object_array_t *
objarr_movef(object_array_t* self, object_t *obj);

object_t *
objarr_popb(object_array_t *self);
