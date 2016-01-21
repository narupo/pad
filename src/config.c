#include "config.h"

struct Config {
	char configdirpath[FILE_NPATH];
	ConfigSetting* setting;
};

/************
* Variables *
************/

static Config* config;  // Singleton instance
static pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for singleton instance of Config

static char const* CONFIG_DIR_PATH = "~/.cap";  // Root directory path of config
static char const* CONFIGSETTING_PATH = "~/.cap/setting";  // File path of config-setting
static char const* CONFIGSETTING_FNAME = "setting";  // File name of config-setting

/***************
* Mutex family *
***************/

static bool
self_lock(void) {
	return pthread_mutex_lock(&config_mutex) == 0;
}

static bool
self_unlock(void) {
	return pthread_mutex_unlock(&config_mutex) == 0;
}

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
 * @param[in] configdirpath source directory of construct
 *
 * @return success to pointer to Config
 * @return failed to pointer to NULL 
 */
static Config*
config_new_from_dir(char const* configdirpath) {
	// Construct
	Config* self = (Config*) calloc(1, sizeof(Config));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	// Make root path from constant because constant path is not solved
	if (!file_solve_path(self->configdirpath, NUMOF(self->configdirpath), configdirpath)) {
		WARN("Failed to solve path \"%s\"", configdirpath);
		free(self);
		return NULL;
	}

	// Solve root directory path
	char sdirpath[FILE_NPATH];

	if (!file_solve_path(sdirpath, sizeof sdirpath, self->configdirpath)) {
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
	char fname[FILE_NPATH];
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
	return config_new_from_dir(CONFIG_DIR_PATH);
}

/**
 * Destroy instance of Config
 */
static void
config_destroy(void) {
	if (config) {
		config_delete(config);
		config = NULL;
	}
}

Config*
config_instance(void) {
	if (self_lock()) {
		if (!config) {
			config = config_new();
			atexit(config_destroy);
		}
		self_unlock();
		return config;
	}

	return NULL;
}

/*********
* Getter *
*********/

char const*
config_dir(Config const* self) {
	return self->configdirpath;
}

char const*
config_path(Config const* self, char const* key) {
	if (self_lock()) {
		char const* path = configsetting_path(self->setting, key);
		self_unlock();
		return path;
	}

	return NULL;
}

static char*
config_path_with(Config const* self, char* dst, size_t dstsize, char const* with, char const* base) {
	// Check arguments
	if (!self || !dst || !base) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Get cap's current directory path
	char const* withpath = NULL;

	if (self_lock()) {
		withpath = configsetting_path(self->setting, with);
		if (!withpath) {
			WARN("Not found \"%s\" in setting", with);
			*dst = '\0';
			self_unlock();
			return dst;
		}
		self_unlock();
	}

	// Make path
	char tmp[dstsize];
	snprintf(tmp, dstsize, "%s/%s", withpath, base);

	// Solve path
	if (!file_solve_path(dst, dstsize, tmp)) {
		WARN("Failed to solve path \"%s\"", tmp);
		return NULL;
	}

	// Is out of home?
	if (config_is_out_of_home(self, dst)) {
		// Yes, set path to home
		snprintf(dst, dstsize, config_path(self, "home"));
	}

	return dst;
}

char*
config_path_with_cd(Config const* self, char* dst, size_t dstsize, char const* base) {
	return config_path_with(self, dst, dstsize, "cd", base);
}

char*
config_path_with_home(Config const* self, char* dst, size_t dstsize, char const* base) {
	return config_path_with(self, dst, dstsize, "home", base);
}

char*
config_make_path_from_base(Config const* self, char const* basename) {
	// Check arguments
	if (!basename) {
		WARN("Invalid arguments");
		return NULL;
	}

	char* dst = (char*) calloc(FILE_NPATH, sizeof(char));
	if (!dst) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	return config_path_with_cd(self, dst, FILE_NPATH, basename);
}

/*********
* Setter *
*********/

bool
config_set_path(Config* self, char const* key, char const* val) {
	if (self_lock()) {
		bool res = configsetting_set_path(self->setting, key, val);
		self_unlock();
		return res;
	}

	return false;
}

bool
config_save(Config const* self) {
	if (self_lock()) {
		bool res = configsetting_save_to_file(self->setting, CONFIGSETTING_PATH);
		self_unlock();
		return res;
	}
	
	return false;
}

// TODO: FIXME
// This logic was failed
bool
config_is_out_of_home(Config const* self, char const* path) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path of \"%s\"", path);
		return false;
	}

	char const* home = config_path(self, "home");
	size_t homelen = strlen(home);
	size_t pathlen = strlen(spath);

	if (pathlen >= homelen && strncmp(spath, home, homelen) == 0) {
		return false;
	}

	return true;
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
