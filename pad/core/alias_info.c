#include <pad/core/alias_info.h>

struct PadAliasInfo {
    PadDict *key_val_map;
    PadDict *key_desc_map;
};

void
PadAliasInfo_Del(PadAliasInfo *self) {
    if (!self) {
        return;
    }

    PadDict_Del(self->key_val_map);
    PadDict_Del(self->key_desc_map);
    free(self);
}

PadAliasInfo *
PadAliasInfo_New(void) {
    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = PadDict_New(128);
    if (!self->key_val_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_New(128);
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

    self->key_val_map = PadDict_DeepCopy(other->key_val_map);
    if (!self->key_val_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_DeepCopy(other->key_desc_map);
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

    self->key_val_map = PadDict_ShallowCopy(other->key_val_map);
    if (!self->key_val_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_ShallowCopy(other->key_desc_map);
    if (!self->key_desc_map) {
        PadAliasInfo_Del(self);
        return NULL;
    }

    return self;  
}

const char *
PadAliasInfo_GetcValue(const PadAliasInfo *self, const char *key) {
   const PadDictItem *item = PadDict_Getc(self->key_val_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

const char *
PadAliasInfo_GetcDesc(const PadAliasInfo *self, const char *key) {
   const PadDictItem *item = PadDict_Getc(self->key_desc_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

PadAliasInfo *
PadAliasInfo_SetValue(PadAliasInfo *self, const char *key, const char *value) {
    PadDict *result = PadDict_Set(self->key_val_map, key, value);
    if (!result) {
        return NULL;
    }

    return self;
}

PadAliasInfo *
PadAliasInfo_SetDesc(PadAliasInfo *self, const char *key, const char *desc) {
    PadDict *result = PadDict_Set(self->key_desc_map, key, desc);
    if (!result) {
        return NULL;
    }

    return self;
}

void
PadAliasInfo_Clear(PadAliasInfo *self) {
    PadDict_Clear(self->key_val_map);
    PadDict_Clear(self->key_desc_map);
}

const PadDict *
PadAliasInfo_GetcKeyValueMap(const PadAliasInfo *self) {
    return self->key_val_map;
}

const PadDict *
PadAliasInfo_GetcKeyDescMap(const PadAliasInfo *self) {
    return self->key_desc_map;
}
