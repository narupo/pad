#include "config.h"

enum {
	CONFIG_FORMAT_SIZE = 256,
};

struct Config {
	char dirpath[FILE_NPATH];
	char filepath[FILE_NPATH];
	char homesdirpath[FILE_NPATH];
	char curhomedirpath[FILE_NPATH];
	char curtrashdirpath[FILE_NPATH];
	Json* json;
};

/************
* Variables *
************/

static Config* config; // Singleton instance
static pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for singleton instance of Config

static const char PROGNAME[] = "cap config";
static const char CONFIG_DIR_PATH[] = "~/.cap"; // Root directory path of config
static const char CONFIG_FNAME[] = "config"; // File name of config-setting
static const char CONFIG_HOMES_DIRNAE[] = "homes";
static const char CONFIG_TRASH_DIRNAE[] = "trash";

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

static const char*
self_path_unsafe(const Config* self, const char* key) {
	// Find from json
	JsonObject* root = json_root(self->json);
	String const* s = jsonobj_find_value(root, key);
	if (!s) {
		WARN("Failed to find value \"%s\"", key);
		return "";
	}

	// Done
	return str_get_const(s);
}

static bool
self_is_out_of_home_unsafe(const Config* self, const char* path) {
	char spath[FILE_NPATH];
	if (!file_solve_path(spath, sizeof spath, path)) {
		WARN("Failed to solve path of \"%s\"", path);
		return true;
	}

	const char* home = self_path_unsafe(config, "home");
	char shome[FILE_NPATH];
	if (!file_solve_path(shome, sizeof shome, home)) {
		WARN("Failed to solve path of \"%s\"", home)
		return true;
	}

	size_t homelen = strlen(shome);
	size_t pathlen = strlen(spath);

	if (strncmp(spath, shome, homelen) != 0 || pathlen < homelen) {
		return true;
	}

	return false;
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
config_init_file_unsafe(Config* self, const char* fname) {
	// Cross-platform for Linux and MS Windows
#if defined(_CAP_WINDOWS)
	static const char DEFAULT_HOME_PATH[] = "C:/Windows/Temp";
	static const char DEFAULT_EDITOR_PATH[] = "C:/Windows/notepad.exe"; // wtf
#else
	static const char DEFAULT_HOME_PATH[] = "/tmp";
	static const char DEFAULT_EDITOR_PATH[] = "/usr/bin/vi";
#endif

	// Ready
	char fmt[CONFIG_FORMAT_SIZE];
	String* strbuf = str_new();
	if (!strbuf) {
		WARN("Failed to construct string");
		goto fail;
	}

	// Create save buffer with JSON format
	str_append_string(strbuf, "{\n");
	str_append_nformat(strbuf, fmt, sizeof fmt, "	\"home\": \"%s\",\n", DEFAULT_HOME_PATH);
	str_append_nformat(strbuf, fmt, sizeof fmt, "	\"cd\": \"%s\",\n", DEFAULT_HOME_PATH);
	str_append_nformat(strbuf, fmt, sizeof fmt, "	\"editor\": \"%s\"\n", DEFAULT_EDITOR_PATH);
	str_append_string(strbuf, "}\n");

	// Save
	FILE* fout = file_open(fname, "wb");
	if (!fout) {
		WARN("Failed to open file \"%s\"", fname);
		goto fail;
	}

	const char* savebuf = str_get_const(strbuf);
	size_t buflen = str_length(strbuf);
	if (!fwrite(savebuf, sizeof(savebuf[0]), buflen, fout)) {
		WARN("Failed to write file \"%s\"", fname);
		fclose(fout);
		goto fail;
	}

	if (fclose(fout) != 0) {
		WARN("Failed to close file \"%s\"", fname);
		goto fail;
	}

	// Read config from saved file
	if (!json_read_from_file(self->json, fname)) {
		WARN("Failed to read from file \"%s\"", fname);
		goto fail;
	}

	str_delete(strbuf);
	return self;

fail:
	str_delete(strbuf);
	return NULL;
}

static bool
not_exists_to_mkdir(const char* path) {
	if (file_is_exists(path)) {
		return true;
	}

	if (file_mkdir_mode(path, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
		return false;
	}

	return true;
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
config_new_from_dir_unsafe(const char* srcdirpath) {
	// Construct
	Config* self = (Config*) calloc(1, sizeof(Config));
	if (!self) {
		WARN("Failed to allocate memory");
		goto fail;
	}

	/***************
	* Create paths *
	***************/

	// Root directory path
	if (!file_solve_path(self->dirpath, sizeof self->dirpath, srcdirpath)) {
		WARN("Failed to solve path \"%s\"", srcdirpath);
		goto fail;
	}

	// Config file path
	if (!file_solve_path_format(self->filepath, sizeof self->filepath, "%s/%s", srcdirpath, CONFIG_FNAME)) {
		WARN("Failed to solve path \"%s/%s\"", srcdirpath, CONFIG_FNAME);
		goto fail;
	}

	// Homes directory path
	if (!file_solve_path_format(self->homesdirpath, sizeof self->homesdirpath, "%s/%s", srcdirpath, CONFIG_HOMES_DIRNAE)) {
		WARN("Failed to solve path \"%s/%s\"", srcdirpath, CONFIG_HOMES_DIRNAE);
		goto fail;
	}

	/********************
	* Create directorys *
	********************/

	// Root directory
	if (!not_exists_to_mkdir(self->dirpath)) {
		WARN("Failed to make cap directory \"%s\"", self->dirpath);
		goto fail;
	}

	// Homes directory
	if (!not_exists_to_mkdir(self->homesdirpath)) {
		WARN("Failed to make alias directory \"%s\"", self->homesdirpath);
		goto fail;
	}

	/*******
	* Load *
	*******/

	// Load config from directory
	self->json = json_new();
	if (!self->json) {
		WARN("Failed to construct json");
		goto fail;
	}

	if (!file_is_exists(self->filepath)) {
		if (!config_init_file_unsafe(self, self->filepath)) {
			WARN("Failed to init file \"%s\"", self->filepath);
			goto fail;
		}
	} else {
		if (!json_read_from_file(self->json, self->filepath)) {
			WARN("Failed to read from file \"%s\"", self->filepath);
			json_delete(self->json);
			goto fail;
		}
	}

	/********************************************
	* Initialize current hash directory of home *
	********************************************/

	// Paths

	const char* home = self_path_unsafe(self, "home");

	if (!file_solve_path_format(self->curhomedirpath, sizeof self->curhomedirpath, "%s/%d", self->homesdirpath, hash_int_from_path(home))) {
		WARN("Failed to solve path of current home directory");
		goto fail;
	}

	if (!file_solve_path_format(self->curtrashdirpath, sizeof self->curtrashdirpath, "%s/%s", self->curhomedirpath, CONFIG_TRASH_DIRNAE)) {
		WARN("Failed to solve path of current trash directory");
		goto fail;
	}

	// Directorys

	if (!not_exists_to_mkdir(self->curhomedirpath)) {
		WARN("Failed to make directory \"%s\"", self->curhomedirpath);
		goto fail;
	}

	if (!not_exists_to_mkdir(self->curtrashdirpath)) {
		WARN("Failed to make directory \"%s\"", self->curtrashdirpath);
		goto fail;
	}

	// Done
	return self;

fail:
	json_delete(self->json);
	free(self);
	return NULL;
}

/**
 * Construct Config
 *
 * @return success to pointer to Config
 * @return failed to pointer to NULL
 */
static Config*
config_new_unsafe(void) {
	return config_new_from_dir_unsafe(CONFIG_DIR_PATH);
}

/**
 * Destroy instance of Config
 */
static void
config_destroy_unsafe(void) {
	if (config) {
		config_delete(config);
		config = NULL;
	}
}

Config*
config_instance(void) {
	if (self_lock()) {
		if (!config) {
			config = config_new_unsafe();
			atexit(config_destroy_unsafe);
		}
		self_unlock();
		return config;
	}

	return NULL;
}

/*********
* Getter *
*********/

static const char*
config_dirpath_unsafe(const Config* self, const char* key) {
	const char* path = NULL;

	if (strcasecmp(key, "root") == 0) {
		path = self->dirpath;
	} else if (strcasecmp(key, "homes") == 0) {
		path = self->homesdirpath;
	} else if (strcasecmp(key, "home") == 0) {
		path = self->curhomedirpath;
	} else if (strcasecmp(key, "trash") == 0) {
		path = self->curtrashdirpath;
	} else {
		caperr(PROGNAME, CAPERR_NOTFOUND, "key \"%s\"", key);
	}

	return path;
}

const char*
config_dirpath(const Config* self, const char* key) {
	const char* path = NULL;

	if (!self_lock()) {
		caperr(PROGNAME, CAPERR_MUTEX_LOCK, "");
		return path;
	}

	path = config_dirpath_unsafe(self, key);

	if (!self_unlock()) {
		caperr(PROGNAME, CAPERR_MUTEX_UNLOCK, "");
		return path;
	}

	return path;
}

char*
config_make_dirpath(const Config* self, const char* key) {
	char* path = NULL;

	if (!self_lock()) {
		caperr(PROGNAME, CAPERR_MUTEX_LOCK, "");
		return path;
	}

	path = strdup(config_dirpath_unsafe(self, key));

	if (!self_unlock()) {
		caperr(PROGNAME, CAPERR_MUTEX_UNLOCK, "");
		return path;
	}

	return path;
}

char*
config_dirpath_with(const Config* self, char* dst, size_t dstsz, const char* with, const char* name) {

	if (!dst || dstsz == 0 || !with || !name) {
		caperr(PROGNAME, CAPERR_INVALID_ARGUMENTS, "");
		return NULL;
	}

	*dst = '\0';

	const char* dirpath = config_dirpath(self, with);
	if (!dirpath) {
		return NULL;
	}

	if (!self_lock()) {
		caperr(PROGNAME, CAPERR_MUTEX_LOCK, "");
		return NULL;
	}

	if (!file_solve_path_format(dst, dstsz, "%s/%s", dirpath, name)) {
		caperr(PROGNAME, CAPERR_SOLVE, "name of \"%s\"", name);
		*dst = '\0';
		self_unlock();
		return NULL;
	}

	if (!self_unlock()) {
		caperr(PROGNAME, CAPERR_MUTEX_UNLOCK, "");
		return NULL;
	}

	return dst;
}

const char*
config_filepath(const Config* self, const char* key) {
	if (strcasecmp(key, "config") == 0) {
		return self->filepath;
	}

	caperr(PROGNAME, CAPERR_NOTFOUND, "key \"%s\"", key);
	return NULL;
}

const char*
config_path(const Config* self, const char* key) {
	if (self_lock()) {
		const char* path = self_path_unsafe(self, key);
		self_unlock();
		return path; // TODO: Need return copy for multi-thread. Reference is danger.
	}

	return NULL;
}

char*
config_path_with(const Config* self, char* dst, size_t dstsize, const char* with, const char* base) {
	if (!self_lock()) {
		caperr(PROGNAME, CAPERR_MUTEX_LOCK, "\"%s/%s\"", with, base);
		return NULL;
	}

	// Check arguments
	if (!self || !dst || !base) {
		caperr(PROGNAME, CAPERR_INVALID, "arguments");
		goto fail;
	}

	// Get cap's current directory path
	const char* withpath = NULL;

	withpath = self_path_unsafe(self, with);
	if (!withpath) {
		caperr(PROGNAME, CAPERR_NOTFOUND, "\"%s\" in setting", with);
		*dst = '\0';
		goto fail;
	}

	// Make solve path
	if (!file_solve_path_format(dst, dstsize, "%s/%s", withpath, base)) {
		caperr(PROGNAME, CAPERR_SOLVE, "path \"%s/%s\"", withpath, base);
		goto fail;
	}

	// Is out of home?
	if (self_is_out_of_home_unsafe(self, dst)) {
		// Yes, set path to home
		snprintf(dst, dstsize, "%s", self_path_unsafe(self, "home"));
	}

	// Done
	self_unlock();
	return dst;

fail:
	self_unlock();
	return NULL;
}

char*
config_path_with_cd(const Config* self, char* dst, size_t dstsize, const char* base) {
	return config_path_with(self, dst, dstsize, "cd", base);
}

char*
config_path_with_home(const Config* self, char* dst, size_t dstsize, const char* base) {
	return config_path_with(self, dst, dstsize, "home", base);
}

/*********
* Setter *
*********/

bool
config_set_path(Config* self, const char* key, const char* val) {
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

		// Done
		self_unlock();
		return ret;
	}

	return false;
}

bool
config_save(const Config* self) {
	if (self_lock()) {
		bool ret = true;
		ret = json_write_to_file(self->json, self->filepath);
		self_unlock();
		return ret;
	}

	return false;
}

bool
config_is_out_of_home(const Config* self, const char* path) {
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

#if defined(_TEST_CONFIG)
static int
test_server(int argc, char* argv[]) {
	return 0;
}

static int
test_io(int argc, char* argv[]) {
	Config* config = config_instance();
	if (!config) {
		die("config");
	}

	printf("cd[%s]\n", self_path_unsafe(config, "cd"));

	config_set_path(config, "cd", "/tmp");
	printf("cd[%s]\n", self_path_unsafe(config, "cd"));

	config_save(config);
	return 0;
}

static int
test_is_out_of_home(int argc, char* argv[]) {
	if (argc < 2) {
		die("need path");
	}

	Config* config = config_instance();
	if (!config) {
		die("config");
	}

	char dst[FILE_NPATH];
	config_path_with_home(config, dst, sizeof dst, argv[1]);
	return 0;
}

int
main(int argc, char* argv[]) {
	int ret = 0;
    // ret = test_io(argc, argv);
    // ret = test_server(argc, argv);
    ret = test_is_out_of_home(argc, argv);

    fflush(stdout);
    fflush(stderr);
    return ret;
}
#endif
