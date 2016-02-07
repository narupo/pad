#include "config.h"

struct Config {
	char dirpath[FILE_NPATH];
	Json* json;
};

/************
* Variables *
************/

static Config* config;  // Singleton instance
static pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for singleton instance of Config

static char const CONFIG_DIR_PATH[] = "~/.cap";  // Root directory path of config
static char const CONFIGSAVE_PATH[] = "~/.cap/config";  // File path of config-setting
static char const CONFIGSAVE_FNAME[] = "config";  // File name of config-setting

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

static char const*
self_path_unsafe(Config const* self, char const* key) {
	JsonObject* root = json_root(self->json);
	String const* s = jsonobj_find_value(root, key);
	if (!s) {
		WARN("Failed to find value \"%s\"", key);
		return "";
	}
	return str_get_const(s);
}

static bool
self_is_out_of_home_unsafe(Config const* self, char const* path) {
	char spath[FILE_NPATH];

	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path of \"%s\"", path);
		return false;
	}

	char const* home = self_path_unsafe(self, "home");
	size_t homelen = strlen(home);
	size_t pathlen = strlen(spath);

	if (pathlen >= homelen && strncmp(spath, home, homelen) == 0) {
		return false;
	}

	return true;
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
		json_delete(self->json);
		free(self);
	}
}

static Config*
config_init_file(Config* self, char const* fname) {
#if defined(_WIN32) || defined(_WIN64)
	char const DEFAULT_HOME_PATH[] = "C:/Windows/Temp";
	char const DEFAULT_EDITOR_PATH[] = "C:/Windows/notepad.exe";
#else
	char const DEFAULT_HOME_PATH[] = "/tmp";
	char const DEFAULT_EDITOR_PATH[] = "/usr/bin/vi";
#endif

	String* src = str_new();

	str_append_string(src, "{\n");

		str_append_string(src, "	\"home\": \"");
		str_append_string(src, DEFAULT_HOME_PATH);
		str_append_string(src, "\",\n");

		str_append_string(src, "	\"cd\": \"");
		str_append_string(src, DEFAULT_HOME_PATH);
		str_append_string(src, "\",\n");

		str_append_string(src, "	\"editor\": \"");
		str_append_string(src, DEFAULT_EDITOR_PATH);
		str_append_string(src, "\"\n");
		
	str_append_string(src, "}\n");

	FILE* fout = file_open(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		return NULL;
	}

	char const* buf = str_get_const(src);
	size_t buflen = str_length(src);
	if (!fwrite(buf, sizeof(buf[0]), buflen, fout)) {
		WARN("Failed to write file \"%s\"", fname);
		fclose(fout);
		return NULL;
	}

	if (fclose(fout) != 0) {
		WARN("Failed to close file \"%s\"", fname);
		return NULL;
	}

	if (!json_read_from_file(self->json, fname)) {
		WARN("Failed to read from file \"%s\"", fname);
		return NULL;
	}

	return self;
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

	// Make root path from constant because constant path is not solved
	if (!file_solve_path(self->dirpath, NUMOF(self->dirpath), dirpath)) {
		WARN("Failed to solve path \"%s\"", dirpath);
		free(self);
		return NULL;
	}

	// Solve root directory path
	char sdirpath[FILE_NPATH];

	if (!file_solve_path(sdirpath, sizeof sdirpath, self->dirpath)) {
		free(self);
		return NULL;
	}

	// Check directory
	if (!file_is_exists(sdirpath)) {
		if (file_mkdir_mode(sdirpath, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
			WARN("Failed to mkdir \"%s\"", sdirpath);
			free(self);
			return NULL;
		}
	}

	// Load config from directory
	char fname[FILE_NPATH];
	snprintf(fname, sizeof fname, "%s/%s", sdirpath, CONFIGSAVE_FNAME);

	self->json = json_new();

	if (!file_is_exists(fname)) {
		if (!config_init_file(self, fname)) {
			WARN("Failed to init file \"%s\"", fname);
			free(self);
			return NULL;
		}
	} else {
		if (!json_read_from_file(self->json, fname)) {
			WARN("Failed to read from file \"%s\"", fname);
			json_delete(self->json);
			free(self);
			return NULL;
		}
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
	return self->dirpath;
}

char const*
config_path(Config const* self, char const* key) {
	if (self_lock()) {
		char const* path = self_path_unsafe(self, key);
		self_unlock();
		return path;
	}

	return NULL;
}

static char*
config_path_with(Config const* self, char* dst, size_t dstsize, char const* with, char const* base) {
	if (!self_lock()) {
		return NULL;
	}

	// Check arguments
	if (!self || !dst || !base) {
		WARN("Invalid arguments");
		self_unlock();
		return NULL;
	}

	// Get cap's current directory path
	char const* withpath = NULL;

	withpath = self_path_unsafe(self, with);
	if (!withpath) {
		WARN("Not found \"%s\" in setting", with);
		*dst = '\0';
		self_unlock();
		return dst;
	}

	// Make path
	char tmp[dstsize];
	snprintf(tmp, dstsize, "%s/%s", withpath, base);

	// Solve path
	if (!file_solve_path(dst, dstsize, tmp)) {
		WARN("Failed to solve path \"%s\"", tmp);
		self_unlock();
		return NULL;
	}

	// Is out of home?
	if (self_is_out_of_home_unsafe(self, dst)) {
		// Yes, set path to home
		snprintf(dst, dstsize, self_path_unsafe(self, "home"));
	}

	self_unlock();
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

/*********
* Setter *
*********/

bool
config_set_path(Config* self, char const* key, char const* val) {
	if (self_lock()) {
		bool ret = true;

		JsonObject* root = json_root(self->json);
		String* str = jsonobj_find_value(root, key);
		if (!str) {
			WARN("Invalid key \"%s\"", key);
			ret = false;
		} else {
			char path[FILE_NPATH];
			file_solve_path(path, sizeof path, val);
			str_set_string(str, path);
		}
		self_unlock();
		return ret;
	}

	return false;
}

bool
config_save(Config const* self) {
	if (self_lock()) {
		bool ret = true;
		char path[FILE_NPATH];
		file_solve_path(path, sizeof path, CONFIGSAVE_PATH);
		ret = json_write_to_file(self->json, path);
		self_unlock();
		return ret;
	}
	
	return false;
}

bool
config_is_out_of_home(Config const* self, char const* path) {
	if (self_lock()) {
		bool ret = self_is_out_of_home_unsafe(self, path);
		self_unlock();
		return ret;
	}

	return false;
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

	printf("cd[%s]\n", self_path_unsafe(config, "cd"));

	config_set_path(config, "cd", "/tmp");
	printf("cd[%s]\n", self_path_unsafe(config, "cd"));

	config_save(config);
	
	config_delete(config);
    return 0;
}
#endif
