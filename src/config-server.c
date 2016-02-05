#include "config-server.h"

/***************
* ConfigServer *
***************/

/**********************************
* ConfigServer constant variables *
**********************************/

#if defined(_WIN32) || defined(_WIN64)
#else
#endif

static char const CONFIGSERVER_DEFAULT_HOSTPORT[] = "127.0.0.1:1234";
static char
static int const LINE_FORMAT_DELIM = ',';

struct ConfigServer {
	StringHashMap* pathmap;
};

/*********
* Stream *
*********/

static bool
self_parse_read_line(ConfigServer* self, char const* line) {
	// Parse read line
	CsvLine* csvline = csvline_new_parse_line(line, LINE_FORMAT_DELIM);
	if (!csvline) {
		WARN("Failed to construct csvline");
		return false;
	}

	// Check parse results
	if (csvline_length(csvline) != 2) {
		WARN("Invalid line format \"%s\"", line);
		csvline_delete(csvline);
		return false;
	}

	// Set key and value
	char const* key = csvline_get_const(csvline, 0);
	char const* val = csvline_get_const(csvline, 1);
	if (!strmap_set_copy(self->pathmap, key, val)) {
		WARN("Failed to pathmap set copy key=\"%s\" val=\"%s\"", key, val);
		csvline_delete(csvline);
		return false;
	}

	// Done
	csvline_delete(csvline);
	return true;
}

static bool
self_load_from_file(ConfigServer* self, char const* fname) {
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
self_create_file(ConfigServer* self, char const* fname) {
	// Create file
	FILE* fout = file_open(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		return false;
	}
	
	// Write lines to file

	// Done
	file_close(fout);
	return true;
}

/*****************
* Delete and New *
*****************/

void
configserver_delete(ConfigServer* self) {
	if (self) {
		strmap_delete(self->pathmap);
		free(self);
	}
}

ConfigServer*
configserver_new_from_file(char const* fname) {
	// Construct
	ConfigServer* self = (ConfigServer*) calloc(1, sizeof(ConfigServer));
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
configserver_path(ConfigServer const* self, char const* key) {
	return strmap_get_const(self->pathmap, key);
}

/*********
* Setter *
*********/

bool
configserver_save_to_file(ConfigServer* self, char const* fname) {
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
configserver_set_path(ConfigServer* self, char const* key, char const* val) {
}

/*******
* Test *
*******/

#if defined(TEST_CONFIGSERVER)
int
main(int argc, char* argv[]) {
	ConfigServer* server = configserver_new_from_file("~/.cap/server");
	configserver_delete(server);
    return 0;
}
#endif
