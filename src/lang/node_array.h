#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <lib/memory.h>
#include <lang/types.h>
#include <lang/nodes.h>

/*****************
* delete and new *
*****************/

void
nodearr_del(node_array_t *self);

node_array_t *
nodearr_new(void);

node_array_t *
nodearr_new_other(node_array_t *other);

/*********
* getter *
*********/

int32_t
nodearr_len(const node_array_t *self);

int32_t
nodearry_capa(const node_array_t *self);

node_t *
nodearr_get(const node_array_t *self, int32_t index);

const node_t *
nodearr_getc(const node_array_t *self, int32_t index);

/*********
* setter *
*********/

node_array_t *
nodearr_resize(node_array_t *self, int32_t capa);

node_array_t *
nodearr_moveb(node_array_t *self, node_t *node);

node_array_t *
nodearr_movef(node_array_t *self, node_t *node);

node_t *
nodearr_popb(node_array_t *self);
