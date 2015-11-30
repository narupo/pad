#include "config-setting.h"

struct ConfigSetting {
	char cdpath[NFILE_PATH];
	char editorpath[NFILE_PATH];
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
	// TODO: hashmap
	if (keycmp(line, "cd") == 0) {
		snprintf(self->cdpath, sizeof self->cdpath, "%s", line+3);
	} else if (keycmp(line, "editor") == 0) {
		snprintf(self->editorpath, sizeof self->editorpath, "%s", line+7);
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
	
	fprintf(fout, "cd /tmp\n");
	fprintf(fout, "editor /usr/bin/vi\n");

	file_close(fout);
	return true;
}

/*****************
* Delete and New *
*****************/

void
config_setting_delete(ConfigSetting* self) {
	if (self) {
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

	// Set default values
	snprintf(self->cdpath, sizeof self->cdpath, "cd /tmp");

	// Check file
	if (!file_is_exists(fname)) {
		// Not exists
		// Create file
		if (!self_create_file(self, fname)) {
			WARN("Failed to create file");
			free(self);
			return NULL;
		}
	}

	// Load from file
	if (!self_load_from_file(self, fname)) {
		WARN("Failed to load from file \"%s\"", fname);
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
	if (strcmp(key, "cd") == 0) {
		return self->cdpath;
	} else if (strcmp(key, "editor") == 0) {
		return self->editorpath;
	}
	return NULL;  // Not found key
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

	fprintf(fout, "cd %s\n", self->cdpath);
	fprintf(fout, "editor %s\n", self->editorpath);

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
		snprintf(self->cdpath, sizeof self->cdpath, "%s", spath);
	} else if (keycmp(key, "editor") == 0) {
		snprintf(self->editorpath, sizeof self->editorpath, "%s", spath);
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

