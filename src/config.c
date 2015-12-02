#include "config.h"

struct Config {
	ConfigSetting* setting;
};

/************
* Variables *
************/

static char const* CONFIG_ROOT_PATH = "~/.config/cap"; 
static char const* CONFIGSETTING_FNAME = "setting";
static char const* CONFIGSETTING_PATH = "~/.config/cap/setting";

/*****************
* Delete and New *
*****************/

void
config_delete(Config* self) {
	if (self) {
		configsetting_delete(self->setting);
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
	char sdirpath[NFILE_PATH];

	if (!file_solve_path(sdirpath, sizeof sdirpath, dirpath)) {
		free(self);
		return NULL;
	}

	// Check directory
	if (!file_is_exists(sdirpath)) {
		
		// TODO: file_mkdir(path, "0700");

		if (file_mkdir(sdirpath, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
			WARN("Failed to mkdir \"%s\"", sdirpath);
			free(self);
			return NULL;
		}
	}

	// Load config from directory
	char fname[NFILE_PATH];
	snprintf(fname, sizeof fname, "%s/%s", sdirpath, CONFIGSETTING_FNAME);
	if (!(self->setting = configsetting_new_from_file(fname))) {
		WARN("Failed to construct setting");
		free(self);
		return NULL;
	}

	return self;
}

Config*
config_new(void) {
	return config_new_from_dir(CONFIG_ROOT_PATH);
}

/*********
* Getter *
*********/

char const*
config_path(Config const* self, char const* key) {
	return configsetting_path(self->setting, key);
}

char*
config_path_from_base(Config const* self, char* dst, size_t dstsize, char const* basename) {
	// Check arguments
	if (!self || !dst || !basename) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Get current directory path
	char const* cdpath = configsetting_path(self->setting, "cd");
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
	return configsetting_set_path(self->setting, key, val);
}

bool
config_save(Config const* self) {
	return configsetting_save_to_file(self->setting, CONFIGSETTING_PATH);
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
