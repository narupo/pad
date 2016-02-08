#include "server.h"

typedef struct Server Server;

enum {
	SERVER_NRECV_BUFFER = 1024,
	SERVER_NCOMMAND_LINE = 128,
	SERVER_NTMP_BUFFER = 128,
};

static char const PROGNAME[] = "cap server";
static char const DEFAULT_HOSTPORT[] = "127.0.0.1:1234";

/*********
* thread *
*********/

static int
thread_index_page_by_path(HttpHeader const* header, Socket* client, char const* dirpath) {
	char const* methvalue = httpheader_method_value(header);
	size_t methvallen = strlen(methvalue);

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		return caperr_printf(PROGNAME, CAPERR_OPEN, "directory \"%s\"", dirpath);
	}

	String* content = str_new();
	if (!content) {
		dir_close(dir);
		return caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "string");
	}

	// Content
	str_append_string(content,
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>Index of</title></head>\n"
		"<body>\n"
		"<h1>Index of "
	);
	str_append_string(content, methvalue);
	str_append_string(content, "</h1>\n");

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		char const* name = dirnode_name(node);

		str_append_string(content, "<div><a href='");
		str_append_string(content, methvalue);
		if (methvalue[methvallen-1] != '/') {
			str_append_string(content, "/");
		}
		str_append_string(content, name);
		str_append_string(content, "'>");
		str_append_string(content, name);
		str_append_string(content, "</a></div>\n");

		dirnode_delete(node);
	}

	str_append_string(content,
		"</body>\n"
		"</html>\n"
	);

	// HTTP Response Header
	char contlen[SERVER_NTMP_BUFFER];
	snprintf(contlen, sizeof contlen, "Content-Length: %d\r\n", str_length(content));

	String* response = str_new();

	str_append_string(response,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	str_append_string(response, contlen);
	str_append_string(response, "\r\n");
	str_append_string(response, "\r\n");
	str_append_other(response, content); // Merge content

	// Send
	socket_send_string(client, str_get_const(response));

	// Done
	str_delete(response);
	str_delete(content);
	dir_close(dir);

	return 0;
}

static void
thread_method_get_script(
	  HttpHeader const* header
	, Socket* client
	, char const* defcmdname
	, char const* path) {
	
	char const* cmdname = defcmdname;
	char cmdln[SERVER_NCOMMAND_LINE];
	char scrln[SERVER_NTMP_BUFFER];
	FILE* fin;

	// Try get command name from file
	fin = file_open(path, "rb");
	if (file_read_script_line(scrln, sizeof scrln, fin)) {
		cmdname  = scrln; // Change command name
	}
	file_close(fin);

	// Open process
	snprintf(cmdln, sizeof cmdln, "%s %s", cmdname, path);
	term_eputsf("command line[%s]", cmdln); // debug

	fin = popen(cmdln, "rb");
	if (!fin) {
		WARN("Failed to open process \"%s\"", cmdln);
		return;
	}

	// Content
	String* content = str_new();
	str_append_stream(content, fin);
	file_close(fin);

	// Response header
	char contlen[SERVER_NTMP_BUFFER];
	snprintf(contlen, sizeof contlen, "Content-Length: %d\r\n", str_length(content));

	String* response = str_new();
	
	str_append_string(response,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	str_append_string(response, contlen);
	str_append_string(response, "\r\n");
	str_append_other(response, content);

	// Send
	socket_send_string(client, str_get_const(response));

	// Done
	str_delete(content);
	str_delete(response);
}

static void
thread_method_get_file(HttpHeader const* header, Socket* client, char const* path) {
	// Other
	FILE* fin = file_open(path, "rb");
	if (!fin) {
		WARN("Failed to open file \"%s\"", path);
		return;
	}

	// Content
	String* content = str_new();
	str_append_string(content, "<pre>");
	str_append_stream(content, fin);
	str_append_string(content, "</pre>");

	file_close(fin);

	// Make send buffer
	String* response = str_new();
	char contlen[SERVER_NTMP_BUFFER];
	snprintf(contlen, sizeof contlen, "Content-Length: %d\r\n", str_length(content));

	str_append_string(response,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	str_append_string(response, contlen);
	str_append_string(response, "\r\n");
	str_append_other(response, content);

	// Send
	socket_send_string(client, str_get_const(response));

	// Done
	str_delete(response);
	str_delete(content);
}

static void
thread_method_get(HttpHeader const* header, Socket* client) {
	char const* methval = httpheader_method_value(header);
	Config const* config = config_instance();
	char path[FILE_NPATH];

	config_path_with_home(config, path, sizeof path, methval);
	term_eputsf("Thread %d GET path[%s]", pthread_self(), path); // debug

	// Not found?
	if (!file_is_exists(path)) {
		socket_send_string(client,
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Length: 0\r\n"
			"\r\n"
		);
		return;
	}

	// Directory?
	if (file_is_dir(path)) {
		term_eputsf("dir \"%s\"", path);
		thread_index_page_by_path(header, client, path);
		return;
	}

	// Command with white list for security
	static struct Command {
		char const* suffix; // File suffix name (.py, .php, ...)
		char const* name; // Command name (python, php, ...)
	} cmds[] = {
		{"py", "python3"},
		{"php", "php"},
		{"rb", "ruby"},
		{"sh", "sh"},
		{0},
	};

	char const* suffix = file_suffix(path);
	if (suffix) {
		for (struct Command const* cmd = cmds; cmd->name; ++cmd) {
			if (strcmp(suffix, cmd->suffix) == 0) {
				thread_method_get_script(header, client, cmd->name, path);
				return;
			}
		}
	}

	// Other files
	thread_method_get_file(header, client, path);
}

static void*
thread_main(void* arg) {
	Socket* client = (Socket*) arg;
	pthread_t selfid = pthread_self();

	HttpHeader* header = httpheader_new();
	if (!header) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "HttpHeader");
		return NULL;
	}

	term_eputsf("Thread %d running...", pthread_self());

	for (;;) {
		char buf[SERVER_NRECV_BUFFER];
		int nrecv = socket_recv_string(client, buf, sizeof buf);
		if (nrecv < 0) {
			WARN("Failed to recv");
			break;
		}

		httpheader_parse_request(header, buf);
		char const* methname = httpheader_method_name(header);
		char const* methvalue = httpheader_method_value(header);

		if (strcmp(methname, "GET") == 0) {
			term_eputsf("Thread %d %s %s", selfid, methname, methvalue);
			thread_method_get(header, client);
		} else {
			socket_send_string(client,
				"HTTP/1.1 405 Method Not Allowed\r\n"
				"Content-Length: 0\r\n"
				"\r\n"
			);
		}
	}

	httpheader_delete(header);
	socket_close(client);

	term_eputsf("Done thread %d", pthread_self());
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
	term_eputsf("CapServer is running by \"%s\".", hostport);
	term_eputsf("** Caution!! THIS SERVER DO NOT PUBLISHED ON INTERNET. DANGER! **");

	// Loop
	for (;;) {
		term_eputsf("accept...");

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

	term_eputsf("Thanks.");
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
	// Construct
	Server* server = server_new(argc, argv);
	if (!server) {
		WARN("Failed to construct server");
		return EXIT_FAILURE;
	}

	// Run
	int res = server_run(server);

	// Done
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
