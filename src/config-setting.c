#include "config-setting.h"


/**********
* HashMap *
**********/

typedef char* HashMap_type;
typedef char const* HashMap_const_type;
typedef struct HashMap HashMap;
typedef struct HashMapNode HashMapNode;
typedef struct HashMapIterator HashMapIterator;

enum {
	NHASH = 701,
};

struct HashMapNode {
	char* key;
	HashMap_type value;
};

struct HashMap {
	size_t tablesize;
	HashMapNode* table[NHASH + 1];  // +1 for final nul
};

struct HashMapIterator {
	size_t index;
	size_t tablesize;
	HashMapNode** table;
	HashMapNode* current;
};


/*******
* Hash *
*******/

static long
hashdivl(char const* s, long nhash) {
	long n = 0;
	for (; *s; ++s) {
		n += *s;
	}
	return n % nhash;
}

static long
hashl(char const* s) {
	return hashdivl(s, NHASH);
}

/*******
* Node *
*******/

static void
hashmapnode_delete(HashMapNode* self) {
	if (self) {
		free(self->key);
		free(self->value);
		free(self);
	}
}

static HashMapNode*
hashmapnode_new_copy(char const* key, HashMap_const_type value) {
	HashMapNode* self = (HashMapNode*) calloc(1, sizeof(HashMapNode));
	if (!self) {
		perror("Failed to construct HashMap");
		return NULL;
	}

	self->key = util_strdup(key);
	self->value = util_strdup(value);

	return self;
}

/***********
* Iterator *
***********/

void
hashmapit_delete(HashMapIterator* self) {
	if (self) {
		free(self);
	}
}

HashMapIterator*
hashmapit_new(HashMap* hashmap) {
	HashMapIterator* self = (HashMapIterator*) calloc(1, sizeof(HashMapIterator));
	if (!self) {
		perror("Failed to construct HashMapIterator");
		return NULL;
	}
	
	self->index = 0;
	self->tablesize = hashmap->tablesize;
	self->table = hashmap->table;
	self->current = hashmap->table[0];

	return self;
}

HashMapNode*
hashmapit_begin(HashMapIterator* self) {
	size_t i;
	for (i = self->index; i != self->tablesize; ++i) {
		if (self->table[i]) {
			break;
		}
	}
	self->index = i;
	return self->table[self->index];
}

HashMapNode*
hashmapit_end(HashMapIterator* self) {
	return self->table[self->tablesize];
}

HashMapNode*
hashmapit_current(HashMapIterator* self) {
	return self->current;
}

HashMapNode*
hashmapit_next(HashMapIterator* self) {
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

static void
hashmap_delete(HashMap* self) {
	if (self) {
		for (int i = 0; i < self->tablesize; ++i) {
			if (self->table[i]) {
				hashmapnode_delete(self->table[i]);
				self->table[i] = NULL;
			}
		}
		free(self);
	}
}

static HashMap*
hashmap_new(void) {
	HashMap* self = (HashMap*) calloc(1, sizeof(HashMap));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}
	
	self->tablesize = NHASH;

	return self;
}

/*********
* Getter *
*********/

static HashMap_type
hashmap_get(HashMap* self, char const* key) {
	static HashMap_type dummy = "";

	long i = hashl(key);
	if (!self->table[i]) {
		return dummy;
	}
	return self->table[i]->value;
}

static HashMap_const_type
hashmap_getc(HashMap const* self, char const* key) {
	static HashMap_const_type dummy = "";

	long i = hashl(key);
	if (!self->table[i]) {
		return dummy;
	}
	return self->table[i]->value;
}

/*********
* Setter *
*********/

static bool
hashmap_set_copy(HashMap* self, char const* key, HashMap_const_type val) {
	HashMapNode* setnode = hashmapnode_new_copy(key, val);
	if (!setnode) {
		WARN("Construct HashMapNode");
		return false;
	}

	long i = hashl(key);

	if (self->table[i]) {
		hashmapnode_delete(self->table[i]);
	}
	
	self->table[i] = setnode;
	
	return true;
}




/****************
* ConfigSetting *
****************/




/***********************************
* ConfigSetting constant variables *
***********************************/

#if defined(_WIN32) || defined(_WIN64)
static char const* DEFAULT_CD_PATH = "C:/Windows/Temp";
static char const* DEFAULT_EDITOR_PATH = "C:/Windows/notepad.exe";
#else
static char const* DEFAULT_CD_PATH = "/tmp";
static char const* DEFAULT_EDITOR_PATH = "/usr/bin/vi";
#endif

static int const LINE_FORMAT_DELIM = ',';

struct ConfigSetting {
	HashMap* pathmap;
};

/*********
* Stream *
*********/

static bool
self_parse_read_line(ConfigSetting* self, char const* line) {
	// Parse read line
	CsvLine* csvline = csvline_new_parse_line(line, LINE_FORMAT_DELIM);
	if (!csvline) {
		WARN("Failed to construct csvline");
		return false;
	}

	// Check parse results
	if (csvline_length(csvline) != 2) {
		WARN("Invalid line format \"%s\"", line);
		goto fail_parse;
	}

	// Set key and value
	char const* key = csvline_get_const(csvline, 0);
	char const* val = csvline_get_const(csvline, 1);

	if (!hashmap_set_copy(self->pathmap, key, val)) {
		WARN("Failed to pathmap set copy key=\"%s\" val=\"%s\"", key, val);
		goto fail_set_copy;
	}

	// Done
	csvline_delete(csvline);
	return true;

fail_parse:
	csvline_delete(csvline);
	return false;

fail_set_copy:
	csvline_delete(csvline);
	return false;	
}

static bool
self_load_from_file(ConfigSetting* self, char const* fname) {
	// Open file
	FILE* fin = file_open(fname, "rb");
	if (!fin) {
		WARN("Failed to open file \"%s\"", fname);
		return false;
	}

	// Buffer for read lines
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		fclose(fin);
		return false;
	}
	
	// Read and parse lines
	for (; buffer_getline(buf, fin); ) {
		char const* str = buffer_get_const(buf);
		if (!self_parse_read_line(self, str)) {
			WARN("Failed to parse read line");
			// Nothing todo
		}
	}

	// Done
	buffer_delete(buf);
	fclose(fin);
	return true;
}

/**********
* Creator *
**********/

static bool
self_create_file(ConfigSetting* self, char const* fname) {
	// Create file
	FILE* fout = file_open(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		return false;
	}
	
	// Write lines to file
	fprintf(fout, "cd%c%s\n", LINE_FORMAT_DELIM, DEFAULT_CD_PATH);
	fprintf(fout, "editor%c%s\n", LINE_FORMAT_DELIM, DEFAULT_EDITOR_PATH);

	// Done
	file_close(fout);
	return true;
}

/*****************
* Delete and New *
*****************/

void
configsetting_delete(ConfigSetting* self) {
	if (self) {
		hashmap_delete(self->pathmap);
		free(self);
	}
}

ConfigSetting*
configsetting_new_from_file(char const* fname) {
	// Construct
	ConfigSetting* self = (ConfigSetting*) calloc(1, sizeof(ConfigSetting));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Construct HashMap for path
	self->pathmap = hashmap_new();
	if (!self->pathmap) {
		WARN("Failed to construct pathmap");
		free(self);
		return NULL;
	}

	// Set default values
	hashmap_set_copy(self->pathmap, "cd", DEFAULT_CD_PATH);
	hashmap_set_copy(self->pathmap, "editor", DEFAULT_EDITOR_PATH);

	// Check file
	if (!file_is_exists(fname)) {
		// Not exists
		// Create file
		if (!self_create_file(self, fname)) {
			WARN("Failed to create file");
			hashmap_delete(self->pathmap);
			free(self);
			return NULL;
		}
	}

	// Load from file
	if (!self_load_from_file(self, fname)) {
		WARN("Failed to load from file \"%s\"", fname);
		hashmap_delete(self->pathmap);
		free(self);
		return NULL;
	}

	return self;
}

/*********
* Getter *
*********/

char const*
configsetting_path(ConfigSetting const* self, char const* key) {
	return hashmap_getc(self->pathmap, key);
}

/*********
* Setter *
*********/

bool
configsetting_save_to_file(ConfigSetting* self, char const* fname) {
	// Solve path
	char path[FILE_NPATH];
	
	if (!file_solve_path(path, sizeof path, fname)) {
		WARN("Failed to solve path \"%s\"", fname);
		return false;
	}

	// Open save file
	FILE* fout = file_open(path, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", path);
		return false;
	}

	// Save path-map's key and value
	HashMapIterator* it = hashmapit_new(self->pathmap);

	for (HashMapNode const* cur = hashmapit_begin(it);
		 cur != hashmapit_end(it);
		 cur = hashmapit_next(it)) {

		fprintf(fout, "%s%c%s\n", cur->key, LINE_FORMAT_DELIM, cur->value);
	}

	hashmapit_delete(it);

	// Done
	file_close(fout);
	return true;
}

bool
configsetting_set_path(ConfigSetting* self, char const* key, char const* val) {
	// Solve path
	char sval[FILE_NPATH];

	if (!file_solve_path(sval, sizeof sval, val)) {
		WARN("Failed to solve path");
		return false;
	}

	return hashmap_set_copy(self->pathmap, key, sval);
}

/*******
* Test *
*******/

#if defined(TEST_CONFIGSETTING)
int
main(int argc, char* argv[]) {
	ConfigSetting* setting = configsetting_new_from_file("~/.cap/setting");

	printf("setting cd[%s]\n", configsetting_path(setting, "cd"));

	configsetting_set_path(setting, "cd", "~/tmp");
	printf("setting cd[%s]\n", configsetting_path(setting, "cd"));

	configsetting_save_to_file(setting, "~/.cap/setting");

	configsetting_delete(setting);
    return 0;
}
#endif
