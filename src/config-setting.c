#include "config-setting.h"

/****************
* ConfigSetting *
****************/

/***********************************
* ConfigSetting constant variables *
***********************************/

#if defined(_WIN32) || defined(_WIN64)
static char const DEFAULT_HOME_PATH[] = "C:/Windows/Temp";
static char const DEFAULT_EDITOR_PATH[] = "C:/Windows/notepad.exe";
#else
static char const DEFAULT_HOME_PATH[] = "/tmp";
static char const DEFAULT_EDITOR_PATH[] = "/usr/bin/vi";
#endif

static int const LINE_FORMAT_DELIM = ',';

struct ConfigSetting {
	StringHashMap* pathmap;
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
	if (!strmap_set_copy(self->pathmap, key, val)) {
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
	String* buf = str_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		fclose(fin);
		return false;
	}
	
	// Read and parse lines
	for (; str_getline(buf, fin); ) {
		char const* str = str_get_const(buf);
		if (!self_parse_read_line(self, str)) {
			WARN("Failed to parse read line");
			// Nothing todo
		}
	}

	// Done
	str_delete(buf);
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
	fprintf(fout, "home%c%s\n", LINE_FORMAT_DELIM, DEFAULT_HOME_PATH);
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
		strmap_delete(self->pathmap);
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

	// Construct StringHashMap for path
	self->pathmap = strmap_new();
	if (!self->pathmap) {
		WARN("Failed to construct pathmap");
		free(self);
		return NULL;
	}

	// Set default values
	strmap_set_copy(self->pathmap, "home", DEFAULT_HOME_PATH);
	strmap_set_copy(self->pathmap, "cd", DEFAULT_HOME_PATH);
	strmap_set_copy(self->pathmap, "editor", DEFAULT_EDITOR_PATH);

	// Check file
	if (!file_is_exists(fname)) {
		// Not exists
		// Create file
		if (!self_create_file(self, fname)) {
			WARN("Failed to create file");
			strmap_delete(self->pathmap);
			free(self);
			return NULL;
		}
	}

	// Load from file
	if (!self_load_from_file(self, fname)) {
		WARN("Failed to load from file \"%s\"", fname);
		strmap_delete(self->pathmap);
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
	return strmap_get_const(self->pathmap, key);
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
	StringHashMapIterator* it = strmapit_new(self->pathmap);

	for (StringHashMapNode const* cur = strmapit_begin(it);
		 cur != strmapit_end(it);
		 cur = strmapit_next(it)) {

		char const* key = strmapnode_key_const(cur);
		char const* value = strmapnode_value_const(cur);
		
		if (strlen(key) == 0) {
			continue;
		}

		fprintf(fout, "%s%c%s\n", key, LINE_FORMAT_DELIM, value);
	}

	strmapit_delete(it);

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

	return strmap_set_copy(self->pathmap, key, sval);
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
