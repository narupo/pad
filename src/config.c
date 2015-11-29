#include "config.h"

struct Config {
	ConfigSetting* setting;
};

static char const* CONFIG_SETTING_FNAME = "setting";
static char const* CONFIG_SETTING_PATH = "~/.cap/setting";

/*****************
* Delete and New *
*****************/

void
config_delete(Config* self) {
	if (self) {
		config_setting_delete(self->setting);
		free(self);
	}
}

Config*
config_new_from_dir(char const* dirpath) {
	// Construct
	Config* self = (Config*) calloc(1, sizeof(Config));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	// Solve root directory path
	char spath[NFILE_PATH];

	if (!file_solve_path(spath, sizeof spath, dirpath)) {
		free(self);
		return NULL;
	}

	// Load config from directory
	char fname[NFILE_PATH];
	snprintf(fname, sizeof fname, "%s/%s", spath, CONFIG_SETTING_FNAME);
	if (!(self->setting = config_setting_new_from_file(fname))) {
		WARN("Failed to construct setting");
		free(self);
		return NULL;
	}

	return self;
}

Config*
config_new(void) {
	return config_new_from_dir("~/.cap");
}

/*********
* Getter *
*********/

char const*
config_path(Config const* self, char const* key) {
	return config_setting_path(self->setting, key);
}

char*
config_path_from_base(Config const* self, char* dst, size_t dstsize, char const* basename) {
	// Check arguments
	if (!self || !dst || !basename) {
		die("Invalid arguments");
	}

	// Get current directory path
	char const* cdpath = config_setting_path(self->setting, "cd");
	if (!cdpath) {
		WARN("Not found cd in setting");
		*dst = '\0';
		return dst;
	}

	// Make path
	snprintf(dst, dstsize, "%s/%s", cdpath, basename);

	return dst;
}

/*********
* Setter *
*********/

bool
config_set_path(Config* self, char const* key, char const* val) {
	return config_setting_set_path(self->setting, key, val);
}

bool
config_save(Config const* self) {
	return config_setting_save_to_file(self->setting, CONFIG_SETTING_PATH);
}

/*******
* Test *
*******/

#if defined(TEST_CONFIG)
int
main(int argc, char* argv[]) {
	Config* config = config_new();
	if (!config) {
		die("config");
	}

	printf("cd[%s]\n", config_path(config, "cd"));

	config_set_path(config, "cd", "/tmp");
	printf("cd[%s]\n", config_path(config, "cd"));

	config_save(config);
	
	config_delete(config);
    return 0;
}
#endif

