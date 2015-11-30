#include "config-setting.h"

/**********
* HashMap *
**********/

#include <stdio.h>
#include <stdlib.h>

typedef struct HashMap HashMap;
typedef char* HashMap_type;

enum {
	NHASH = 701,
};

struct HashMap {
	HashMap_type table[NHASH];
};

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

static void
hashmap_delete(HashMap* self) {
	if (self) {
		for (int i = 0; i < NHASH; ++i) {
			if (self->table[i]) {
				free(self->table[i]);
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
	return self;
}

static const HashMap_type
hashmap_get(HashMap* self, char const* key) {
	static const HashMap_type dummy = "";

	long i = hashl(key);
	if (!self->table[i]) {
		return dummy;
	}
	return self->table[i];
}

static void
hashmap_set(HashMap* self, char const* key, HashMap_type val) {
	long i = hashl(key);
	if (self->table[i]) {
		free(self->table[i]);
	}
	self->table[i] = val;
}




/****************
* Configsetting *
****************/

static char const* DEFAULT_CD_PATH = "/tmp";
static char const* DEFAULT_EDITOR_PATH = "/usr/bin/vi";

struct ConfigSetting {
	HashMap* pathmap;
};

/*********
* Stream *
*********/

static int
keycmp(char const* str, char const* key) {
	return strncmp(str, key, strlen(key));
}

static bool
self_parse_read_line(ConfigSetting* self, char const* line) {
	// TODO: pathmap
	if (keycmp(line, "cd") == 0) {
		hashmap_set(self->pathmap, "cd", strdup(line+3));
	} else if (keycmp(line, "editor") == 0) {
		hashmap_set(self->pathmap, "editor", strdup(line+7));
	}
	return true;
}

static bool
self_load_from_file(ConfigSetting* self, char const* fname) {
	// Open file
	FILE* fin = file_open(fname, "rb");
	if (!fin) {
		return false;
	}

	// Buffer for read lines
	Buffer* buf = buffer_new();
	if (!buf) {
		fclose(fin);
		return false;
	}
	
	// Read and parse lines
	for (; buffer_getline(buf, fin); ) {
		char const* str = buffer_getc(buf);
		if (!self_parse_read_line(self, str)) {
			WARN("Failed to parse read line");
		}
	}

	buffer_delete(buf);
	fclose(fin);
	return true;
}

/**********
* Creator *
**********/

static bool
self_create_file(ConfigSetting* self, char const* fname) {
	FILE* fout = file_open(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		return false;
	}
	
	fprintf(fout, "cd %s\n", DEFAULT_CD_PATH);
	fprintf(fout, "editor %s\n", DEFAULT_EDITOR_PATH);

	file_close(fout);
	return true;
}

/*****************
* Delete and New *
*****************/

void
config_setting_delete(ConfigSetting* self) {
	if (self) {
		hashmap_delete(self->pathmap);
		free(self);
	}
}

ConfigSetting*
config_setting_new_from_file(char const* fname) {
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
	hashmap_set(self->pathmap, "cd", strdup(DEFAULT_CD_PATH));
	hashmap_set(self->pathmap, "editor", strdup(DEFAULT_EDITOR_PATH));

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
config_setting_path(ConfigSetting const* self, char const* key) {
	return hashmap_get(self->pathmap, key);
}

/*********
* Setter *
*********/

bool
config_setting_save_to_file(ConfigSetting* self, char const* fname) {
	FILE* fout = file_open(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		return false;
	}

	char const* key = "cd";
	fprintf(fout, "%s %s\n", key, hashmap_get(self->pathmap, key));
	key = "editor";
	fprintf(fout, "%s %s\n", key, hashmap_get(self->pathmap, key));

	file_close(fout);
	return true;
}

bool
config_setting_set_path(ConfigSetting* self, char const* key, char const* val) {
	// Solve path
	char spath[NFILE_PATH];
	if (!file_solve_path(spath, sizeof spath, val)) {
		WARN("Failed to solve path");
		return false;
	}

	// Set path
	if (keycmp(key, "cd") == 0) {
		hashmap_set(self->pathmap, "cd", strdup(spath));
	} else if (keycmp(key, "editor") == 0) {
		hashmap_set(self->pathmap, "editor", strdup(spath));
	} else {
		WARN("Invalid key");
		return false;  // Invalid key
	}

	return true;
}

/*******
* Test *
*******/

#if defined(TEST_CONFIGSETTING)
int
main(int argc, char* argv[]) {
	ConfigSetting* setting = config_setting_new_from_file("~/.cap/setting");

	printf("setting cd[%s]\n", config_setting_path(setting, "cd"));

	config_setting_set_path(setting, "cd", "~/tmp");
	printf("setting cd[%s]\n", config_setting_path(setting, "cd"));

	config_setting_save_to_file(setting, "~/.cap/setting");

	config_setting_delete(setting);
    return 0;
}
#endif

