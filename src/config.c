#include "config.h"

struct Config {
	ConfigSetting* setting;
};

/************
* Variables *
************/

static Config* config;  // Singleton instance
static pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for singleton instance of Config

static char const* CONFIG_ROOT_PATH = "~/.cap";  // Root directory path of config
static char const* CONFIGSETTING_FNAME = "setting";  // File name of config-setting
static char const* CONFIGSETTING_PATH = "~/.cap/setting";  // File paht of config-setting

/*****************
* Delete and New *
*****************/

/**
 * Destruct Config
 */
static void
config_delete(Config* self) {
	if (self) {
		configsetting_delete(self->setting);
		free(self);
	}
}

/**
 * Construct Config from directory
 * 
 * @param[in] dirpath source directory of construct
 *
 * @return success to pointer to Config
 * @return failed to pointer to NULL 
 */
static Config*
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

/**
 * Construct Config
 * 
 * @return success to pointer to Config
 * @return failed to pointer to NULL
 */
static Config*
config_new(void) {
	return config_new_from_dir(CONFIG_ROOT_PATH);
}

/**
 * Destroy instance of Config
 */
static void
config_destroy(void) {
	if (config) {
		config_delete(config);
	}
}

Config*
config_instance(void) {
	if (!config) {
		config = config_new();
		atexit(config_destroy);
	}
	return config;
}

/*********
* Getter *
*********/

static bool
self_lock(void) {
	return pthread_mutex_lock(&config_mutex) == 0;
}

static bool
self_unlock(void) {
	return pthread_mutex_unlock(&config_mutex) == 0;
}

char const*
config_path(Config const* self, char const* key) {
	char const* path = NULL;

	if (self_lock()) {
		path = configsetting_path(self->setting, key);
		self_unlock();
	}

	return path;
}

char*
config_path_from_base(Config const* self, char* dst, size_t dstsize, char const* basename) {
	// Check arguments
	if (!self || !dst || !basename) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Get current directory path
	char const* cdpath = NULL;

	if (self_lock()) {
		cdpath = configsetting_path(self->setting, "cd");
		if (!cdpath) {
			WARN("Not found cd in setting");
			*dst = '\0';
			self_unlock();
			return dst;
		}
		self_unlock();
	}

	// Make path
	snprintf(dst, dstsize, "%s/%s", cdpath, basename);

	return dst;
}

char*
config_make_path_from_base(Config const* self, char const* basename) {
	// Check arguments
	if (!basename) {
		WARN("Invalid arguments");
		return NULL;
	}

	char* dst = (char*) calloc(NFILE_PATH, sizeof(char));
	if (!dst) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	return config_path_from_base(self, dst, NFILE_PATH, basename);
}

/*********
* Setter *
*********/

bool
config_set_path(Config* self, char const* key, char const* val) {
	bool res = false;

	if (self_lock()) {
		res = configsetting_set_path(self->setting, key, val);
		self_unlock();
	}

	return res;
}

bool
config_save(Config const* self) {
	bool res = false;

	if (self_lock()) {
		res = configsetting_save_to_file(self->setting, CONFIGSETTING_PATH);
		self_unlock();
	}
	
	return res;
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
