#include "strmap.h"

struct StringHashMapNode {
	char* key;
	StringHashMap_type value;
};

struct StringHashMap {
	size_t tablesize;
	StringHashMapNode* table[HASH_NHASH + 1];  // +1 for final nul
};

struct StringHashMapIterator {
	size_t index;
	size_t tablesize;
	StringHashMapNode** table;
	StringHashMapNode* current;
};

/*******
* Node *
*******/

void
strmapnode_delete(StringHashMapNode* self) {
if (self) {
		free(self->key);
		free(self->value);
		free(self);
	}
}

StringHashMapNode*
strmapnode_new_copy(const char* key, StringHashMap_const_type value) {
	StringHashMapNode* self = (StringHashMapNode*) calloc(1, sizeof(StringHashMapNode));
	if (!self) {
		perror("Failed to construct StringHashMap");
		return NULL;
	}

	self->key = util_strdup(key);
	self->value = util_strdup(value);

	return self;
}

const char*
strmapnode_key_const(StringHashMapNode const* self) {
	return self->key;
}

const char*
strmapnode_value_const(StringHashMapNode const* self) {
	return self->value;
}

/***********
* Iterator *
***********/

void
strmapit_delete(StringHashMapIterator* self) {
	if (self) {
		free(self);
	}
}

StringHashMapIterator*
strmapit_new(StringHashMap* strmap) {
	StringHashMapIterator* self = (StringHashMapIterator*) calloc(1, sizeof(StringHashMapIterator));
	if (!self) {
		perror("Failed to construct StringHashMapIterator");
		return NULL;
	}

	self->index = 0;
	self->tablesize = strmap->tablesize;
	self->table = strmap->table;
	self->current = strmap->table[0];

	return self;
}

StringHashMapNode*
strmapit_begin(StringHashMapIterator* self) {
	size_t i;
	for (i = self->index; i != self->tablesize; ++i) {
		if (self->table[i]) {
			break;
		}
	}
	self->index = i;
	return self->table[self->index];
}

StringHashMapNode*
strmapit_end(StringHashMapIterator* self) {
	return self->table[self->tablesize];
}

StringHashMapNode*
strmapit_current(StringHashMapIterator* self) {
	return self->current;
}

StringHashMapNode*
strmapit_next(StringHashMapIterator* self) {
	size_t i;
	for (i = self->index + 1; i != self->tablesize; ++i) {
		if (self->table[i]) {
			break;
		}
	}
	self->index = i;
	self->current = self->table[self->index];

	return self->current;
}

/*****************
* Delete and New *
*****************/

void
strmap_delete(StringHashMap* self) {
	if (self) {
		for (int i = 0; i < self->tablesize; ++i) {
			if (self->table[i]) {
				strmapnode_delete(self->table[i]);
				self->table[i] = NULL;
			}
		}
		free(self);
	}
}

StringHashMap*
strmap_new(void) {
	StringHashMap* self = (StringHashMap*) calloc(1, sizeof(StringHashMap));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	self->tablesize = HASH_NHASH;

	return self;
}

/*********
* Getter *
*********/

StringHashMap_type
strmap_get(StringHashMap* self, const char* key) {
	static StringHashMap_type dummy = "";

	long i = hash_long(key);
	if (!self->table[i]) {
		return dummy;
	}
	return self->table[i]->value;
}

StringHashMap_const_type
strmap_get_const(StringHashMap const* self, const char* key) {
	static StringHashMap_const_type dummy = "";

	long i = hash_long(key);
	if (!self->table[i]) {
		return dummy;
	}
	return self->table[i]->value;
}

/*********
* Setter *
*********/

bool
strmap_set_copy(StringHashMap* self, const char* key, StringHashMap_const_type val) {
	StringHashMapNode* setnode = strmapnode_new_copy(key, val);
	if (!setnode) {
		WARN("Construct StringHashMapNode");
		return false;
	}

	long i = hash_long(key);

	if (self->table[i]) {
		strmapnode_delete(self->table[i]);
	}

	self->table[i] = setnode;

	return true;
}

/**************
* strmap test *
**************/

#if defined(TEST_STRINGMAP)
int
main(int argc, char* argv[]) {

    return 0;
}
#endif

