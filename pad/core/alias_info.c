#include <pad/core/alias_info.h>

struct PadAliasInfo {
    dict_t *key_val_map;
    dict_t *key_desc_map;
};

void
PadAliasInfo_Del(PadAliasInfo *self) {
    if (!self) {
        return;
    }

    dict_del(self->key_val_map);
    dict_del(self->key_desc_map);
    free(self);
}

PadAliasInfo *
PadAliasInfo_New(void) {
    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = dict_new(128);
    if (!self->key_val_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = dict_new(128);
    if (!self->key_desc_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    return self;
}

PadAliasInfo *
PadAliasInfo_DeepCopy(const PadAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = dict_deep_copy(other->key_val_map);
    if (!self->key_val_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = dict_deep_copy(other->key_desc_map);
    if (!self->key_desc_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    return self;
}

PadAliasInfo *
PadAliasInfo_ShallowCopy(const PadAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = dict_shallow_copy(other->key_val_map);
    if (!self->key_val_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = dict_shallow_copy(other->key_desc_map);
    if (!self->key_desc_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    return self;  
}

const char *
PadAliasInfo_GetcValue(const PadAliasInfo *self, const char *key) {
   const dict_item_t *item = dict_getc(self->key_val_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

const char *
PadAliasInfo_GetcDesc(const PadAliasInfo *self, const char *key) {
   const dict_item_t *item = dict_getc(self->key_desc_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

PadAliasInfo *
PadAliasInfo_SetValue(PadAliasInfo *self, const char *key, const char *value) {
    dict_t *result = dict_set(self->key_val_map, key, value);
    if (!result) {
        return NULL;
    }

    return self;
}

PadAliasInfo *
PadAliasInfo_SetDesc(PadAliasInfo *self, const char *key, const char *desc) {
    dict_t *result = dict_set(self->key_desc_map, key, desc);
    if (!result) {
        return NULL;
    }

    return self;
}

void
PadAliasInfo_Clear(PadAliasInfo *self) {
    dict_clear(self->key_val_map);
    dict_clear(self->key_desc_map);
}

const dict_t *
PadAliasInfo_GetcKeyValueMap(const PadAliasInfo *self) {
    return self->key_val_map;
}

const dict_t *
PadAliasInfo_GetcKeyDescMap(const PadAliasInfo *self) {
    return self->key_desc_map;
}
