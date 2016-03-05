#include "server.h"

typedef struct Server Server;

enum {
	SERVER_NRECV_BUFFER = 1024 * 5,
	SERVER_NCOMMAND_LINE = 128,
	SERVER_NTMP_BUFFER = 128,
};

static char const PROGNAME[] = "cap server";
static char const DEFAULT_HOSTPORT[] = "127.0.0.1:1234";

/*********
* thread *
*********/

static int
thread_id(void) {
	return pthread_self();
}

#define thread_eputsf(...) { \
	term_ceprintf(TC_YELLOW, TC_BLACK, "Thread %d: ", thread_id()); \
	term_eputsf(__VA_ARGS__); \
}

#define thread_eprintf(...) { \
	term_ceprintf(TC_YELLOW, TC_BLACK, "Thread %d: ", thread_id()); \
	term_eprintf(__VA_ARGS__); \
}

#define thread_ceprintf(fg, bg, ...) { \
	term_ceprintf(TC_YELLOW, TC_BLACK, "Thread %d: ", thread_id()); \
	term_ceprintf(fg, bg, __VA_ARGS__); \
}

static int
thread_index_page_by_path(HttpHeader const* header, Socket* client, char const* dirpath) {
	char const* methvalue = httpheader_method_value(header);
	size_t methvallen = strlen(methvalue);

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		return caperr_printf(PROGNAME, CAPERR_OPEN, "directory \"%s\"", dirpath);
	}

	Buffer* content = buffer_new();
	if (!content) {
		dir_close(dir);
		return caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "buffer");
	}

	// Content
	buffer_append_string(content,
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>Index of</title></head>\n"
		"<body>\n"
		"<h1>Index of "
	);
	buffer_append_string(content, methvalue);
	buffer_append_string(content, "</h1>\n");

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		char const* name = dirnode_name(node);

		buffer_append_string(content, "<div><a href='");
		buffer_append_string(content, methvalue);
		if (methvalue[methvallen-1] != '/') {
			buffer_append_string(content, "/");
		}
		buffer_append_string(content, name);
		buffer_append_string(content, "'>");
		buffer_append_string(content, name);
		buffer_append_string(content, "</a></div>\n");

		dirnode_delete(node);
	}

	buffer_append_string(content,
		"</body>\n"
		"</html>\n"
	);

	// HTTP Response Header
	char contlen[SERVER_NTMP_BUFFER];
	snprintf(contlen, sizeof contlen, "Content-Length: %d\r\n", buffer_length(content));

	Buffer* response = buffer_new();

	buffer_append_string(response,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	buffer_append_string(response, contlen);
	buffer_append_string(response, "\r\n");
	buffer_append_string(response, "\r\n");
	buffer_append_other(response, content); // Merge content

	// Send
	socket_send_bytes(client, buffer_get_const(response), buffer_length(response));

	// Done
	buffer_delete(response);
	buffer_delete(content);
	dir_close(dir);

	thread_ceprintf(TC_GREEN, TC_BLACK, "200 OK\n");
	return 0;
}

static void
thread_method_get_script(
	  HttpHeader const* header
	, Socket* client
	, char const* defcmdname
	, char const* path) {
	
	char const* cmdname = defcmdname;
	char cmdline[SERVER_NCOMMAND_LINE];
	char scriptline[SERVER_NTMP_BUFFER];
	FILE* fin;

	// Try get command name from file
	fin = file_open(path, "rb");
	if (file_read_script_line(scriptline, sizeof scriptline, fin)) {
		cmdname  = scriptline; // Change command name
	}

	if (file_close(fin) != 0) {
		WARN("Faile to close file \"%s\"", path);
		return;
	}

	// Open process
	snprintf(cmdline, sizeof cmdline, "%s %s", cmdname, path);
	thread_eputsf("Command line \"%s\"", cmdline); // debug

	thread_eputsf("Open process...");
	fin = popen(cmdline, "r");
	if (!fin) {
		WARN("Failed to open process \"%s\"", cmdline);
		return;
	}

	// Content
	thread_eputsf("Read from process...");
	Buffer* content = buffer_new();
	buffer_append_stream(content, fin);
	pclose(fin);

	// Response header
	char contlen[SERVER_NTMP_BUFFER];
	snprintf(contlen, sizeof contlen, "Content-Length: %d\r\n", buffer_length(content));
	thread_eputsf("Read content length %d/bytes", buffer_length(content));

	Buffer* response = buffer_new();
	
	buffer_append_string(response,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	buffer_append_string(response, contlen);
	buffer_append_string(response, "\r\n");
	buffer_append_other(response, content);

	// Send
	thread_eputsf("Send response...");
	socket_send_bytes(client, buffer_get_const(response), buffer_length(response));

	// Done
	buffer_delete(content);
	buffer_delete(response);
	thread_ceprintf(TC_GREEN, TC_BLACK, "200 OK\n");
}

static void
thread_method_get_file(
	HttpHeader const* header,
	Socket* client,
	char const* path) {
	
	// Other
	FILE* fin = file_open(path, "rb");
	if (!fin) {
		WARN("Failed to open file \"%s\"", path);
		return;
	}

	// Make content
	Buffer* content = buffer_new();
	buffer_append_stream(content, fin);

	if (file_close(fin) != 0) {
		WARN("Failed to close file \"%s\"", path);
		buffer_delete(content);
		return;
	}

	// Make response with content
	Buffer* response = buffer_new();
	char contlen[SERVER_NTMP_BUFFER];
	snprintf(contlen, sizeof contlen, "Content-Length: %d\r\n", buffer_length(content));

	buffer_append_string(response,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	buffer_append_string(response, contlen);
	buffer_append_string(response, "\r\n");
	buffer_append_other(response, content);

	// Send
	socket_send_bytes(client, buffer_get_const(response), buffer_length(response));

	// Done
	buffer_delete(response);
	buffer_delete(content);
	thread_ceprintf(TC_GREEN, TC_BLACK, "200 OK\n");
}

static void
thread_method_get(
	HttpHeader const* header,
	Socket* client) {

	char const* methval = httpheader_method_value(header);
	Config const* config = config_instance();
	char path[FILE_NPATH];

	// Get path with home
	config_path_with_home(config, path, sizeof path, methval);
	thread_eprintf("Get path \"");
	term_ceprintf(TC_CYAN, TC_BLACK, "%s", path);
	term_eprintf("\"\n");

	// Not found?
	if (!file_is_exists(path)) {
		socket_send_string(client,
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Length: 0\r\n"
			"\r\n"
		);
		thread_ceprintf(TC_RED, TC_BLACK, "404 Not Found\n");
		return;
	}

	// Directory?
	if (file_is_dir(path)) {
		thread_eputsf("Get index list by directory \"%s\"", path);
		thread_index_page_by_path(header, client, path);
		return;
	}

	// Command with white list for security
	JsonObject* servobj = (JsonObject*) config_server_const(config);
	JsonObject* sufobj = jsonobj_find_dict(servobj, "suffix-command");

	char const* suffix = file_suffix(path);
	if (suffix) {
		for (JsonIter it = jsonobj_begin(sufobj), end = jsonobj_end(sufobj);
			!jsoniter_equals(&it, &end);
			jsoniter_next(&it)) {

			JsonObject* obj = jsoniter_value(&it);

			switch (jsonobj_type_const(obj)) {
			default: break;
			case JOTValue: {
				String const* cmd = jsonobj_value(obj);
				String const* suf = jsonobj_name_const(obj);
				if (strcmp(suffix, str_get_const(suf)) == 0) {
					thread_eputsf("Get content by script of \"%s\"", path);
					thread_method_get_script(header, client, str_get_const(cmd), path);
					return;
				}
			} break;
			}
		}
	}

	// Other files
	thread_eputsf("Get content by file of \"%s\"", path);
	thread_method_get_file(header, client, path);
}

static void*
thread_main(void* arg) {
	Socket* client = (Socket*) arg;

	HttpHeader* header = httpheader_new();
	if (!header) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "HttpHeader");
		return NULL;
	}

	thread_eputsf("Created thread");

	for (;;) {
		char buf[SERVER_NRECV_BUFFER];
		int nrecv = socket_recv_string(client, buf, sizeof buf);
		if (nrecv <= 0) {
			WARN("Failed to recv");
			break;
		}

		thread_eprintf("Recv (%d/bytes) \"" , nrecv);
		term_ceprintf(TC_CYAN, TC_BLACK, "%s" , buf);
		term_eprintf("\"\n" , nrecv);

		httpheader_parse_request(header, buf);
		char const* methname = httpheader_method_name(header);
		char const* methvalue = httpheader_method_value(header);
		thread_eprintf("Http method name \"");
		term_ceprintf(TC_YELLOW, TC_BLACK, "%s", methname);
		term_eprintf("\" and value \"");
		term_ceprintf(TC_CYAN, TC_BLACK, "%s", methvalue);
		term_eprintf("\"\n");

		if (strcmp(methname, "GET") == 0) {
			thread_eputsf("Accept GET method");
			thread_method_get(header, client);
		} else {
			socket_send_string(client,
				"HTTP/1.1 405 Method Not Allowed\r\n"
				"Content-Length: 0\r\n"
				"\r\n"
			);
			thread_ceprintf(TC_RED, TC_BLACK, "405 Method Not Allowed");
		}
	}

	httpheader_delete(header);
	socket_close(client);

	thread_ceprintf(TC_GREEN, TC_BLACK, "Done\n");
	return NULL;
}

/*********
* server *
*********/

struct Server {
	int argc;
	int optind;
	char** argv;

	bool opt_is_help;
};

static bool
server_parse_options(Server* self);

static void
server_delete(Server* self) {
	if (self) {
		free(self);
	}
}

static Server*
server_new(int argc, char* argv[]) {
	// Construct
	Server* self = (Server*) calloc(1, sizeof(Server));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Set values
	self->argc = argc;
	self->argv = argv;

	// Parse server options
	if (!server_parse_options(self)) {
		WARN("Failed to parse options");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
server_parse_options(Server* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->opt_is_help = true; break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		WARN("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static void
server_display_welcome_message(Server const* self, char const* hostport) {
	time_t runtime = time(NULL);
	term_ceprintf(TC_GREEN, TC_BLACK,
		"        _____    ___    _______       \n"
		"       /____/\\ /|___|\\ |\\______\\  \n"
		"      /     \\///     \\\\||    __ \\ \n"
		"     |    ==<//   |   \\||    ___|    \n"
		"      \\_____/ \\___^___/\\|___/      \n"
		"                                      \n"
	);
	term_ceprintf(TC_MAGENTA, TC_BLACK,
		"        === Server for dev ===        \n"
		"\n"
	);
	term_ceprintf(TC_GREEN, TC_BLACK, "    CapSurver ");
	term_eprintf("running on ");
	term_ceprintf(TC_YELLOW, TC_BLACK, "%s\n", hostport);
	term_eprintf("    Run at ");
	term_ceprintf(TC_MAGENTA, TC_BLACK, "%s", ctime(&runtime));
	term_ceprintf(TC_RED, TC_BLACK,
		"    ** CAUTION!! THIS SERVER DO NOT PUBLISHED ON INTERNET. **\n\n", hostport);
}

static int
server_run(Server* self) {
	// Update default value
	char const* hostport = DEFAULT_HOSTPORT;
	if (self->argc > self->optind) {
		hostport = self->argv[self->optind];
	}

	// Open socket
	Socket* server = socket_open(hostport, "tcp-server");
	if (!server) {
		return caperr_printf(PROGNAME, CAPERR_OPEN, "socket");
	}

	// Welcome message
	server_display_welcome_message(self, hostport);

	// Loop
	for (;;) {
		thread_eputsf("Accept...");

		Socket* client = socket_accept(server);
		if (!client) {
			caperr_printf(PROGNAME, CAPERR_OPEN, "accept socket");
			continue;
		}
		
		// Thread works
		pthread_t thread;

		if (pthread_create(&thread, NULL, thread_main, (void*) client) != 0) {
			WARN("Failed to create thread");
			continue;
		}

		if (pthread_detach(thread) != 0) {
			WARN("Failed to detach thread");
			continue;
		}
	}

	// Done
	socket_close(server);

	return 0;
}

/**************************
* server public interface *
**************************/

void
server_usage(void) {
    term_eprintf( 
        "Usage:\n"
        "\n"
        "\t%s [host-ip-address:number-of-port] [option]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\n"
    , PROGNAME);
}

int
server_main(int argc, char* argv[]) {
	Server* server = server_new(argc, argv);
	if (!server) {
		WARN("Failed to construct server");
		return EXIT_FAILURE;
	}

	int res = server_run(server);
	
	server_delete(server);
	return res;
}

/**************
* server test *
**************/

#if defined(TEST_SERVER)
int
main(int argc, char* argv[]) {
	return server_main(argc, argv);
}
#endif
