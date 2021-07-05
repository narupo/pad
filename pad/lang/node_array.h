#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/nodes.h>

/*****************
* delete and new *
*****************/

void
nodearr_del(node_array_t *self);

void
nodearr_del_without_nodes(node_array_t* self);

node_array_t *
nodearr_new(void);

node_array_t *
nodearr_deep_copy(const node_array_t *other);

node_array_t *
nodearr_shallow_copy(const node_array_t *other);

/*********
* getter *
*********/

int32_t
nodearr_len(const node_array_t *self);

int32_t
nodearry_capa(const node_array_t *self);

PadNode *
nodearr_get(const node_array_t *self, int32_t index);

const PadNode *
nodearr_getc(const node_array_t *self, int32_t index);

PadNode *
nodearr_get_last(const node_array_t *self);

/*********
* setter *
*********/

node_array_t *
nodearr_resize(node_array_t *self, int32_t capa);

node_array_t *
nodearr_moveb(node_array_t *self, PadNode *node);

node_array_t *
nodearr_movef(node_array_t *self, PadNode *node);

PadNode *
nodearr_popb(node_array_t *self);
