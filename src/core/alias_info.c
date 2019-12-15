#include "core/alias_info.h"

struct alias_info {
    dict_t *key_val_map;
    dict_t *key_desc_map;
};

void
alinfo_del(alinfo_t *self) {
    if (!self) {
        return;
    }

    dict_del(self->key_val_map);
    dict_del(self->key_desc_map);
}

alinfo_t *
alinfo_new(void) {
    alinfo_t *self = mem_ecalloc(1, sizeof(*self));

    self->key_val_map = dict_new(128);
    self->key_desc_map = dict_new(128);

    return self;
}

const char *
alinfo_getc_value(const alinfo_t *self, const char *key) {
   const dict_item_t *item = dict_getc(self->key_val_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

const char *
alinfo_getc_desc(const alinfo_t *self, const char *key) {
   const dict_item_t *item = dict_getc(self->key_desc_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

alinfo_t *
alinfo_set_value(alinfo_t *self, const char *key, const char *value) {
    dict_t *result = dict_set(self->key_val_map, key, value);
    if (!result) {
        return NULL;
    }

    return self;
}

alinfo_t *
alinfo_set_desc(alinfo_t *self, const char *key, const char *desc) {
    dict_t *result = dict_set(self->key_desc_map, key, desc);
    if (!result) {
        return NULL;
    }

    return self;
}

void
alinfo_clear(alinfo_t *self) {
    dict_clear(self->key_val_map);
    dict_clear(self->key_desc_map);
}

const dict_t *
alinfo_getc_key_value_map(const alinfo_t *self) {
    return self->key_val_map;
}

const dict_t *
alinfo_getc_key_desc_map(const alinfo_t *self) {
    return self->key_desc_map;
}
