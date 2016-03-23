#include "server.h"

typedef struct Server Server;

enum {
	SERVER_NRECV_BUFFER = 1024 * 5,
	SERVER_NCOMMAND_LINE = 128,
	SERVER_NTMP_BUFFER = 128,
};

static char const SERVER_NAME[] = "CapServer";
static char const PROGNAME[] = "cap server";
static char const DEFAULT_HOSTPORT[] = "127.0.0.1:1234";

/***********
* response *
***********/

enum {
	RESPONSE_INIT_STATUS = 500,
};

typedef struct Response Response;

struct Response {
	int status;
	Buffer* buffer;
	Buffer* content;
};

void
response_delete(Response* self) {
	if (self) {
		buffer_delete(self->buffer);
		buffer_delete(self->content);
		free(self);
	}
}

Response*
response_new(void) {
	Response* self = (Response*) mem_calloc(1, sizeof(Response));
	if (!self) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "response");
		return NULL;
	}

	self->status = RESPONSE_INIT_STATUS;

	self->buffer = buffer_new();
	if (!self->buffer) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "buffer");
		goto fail;
	}

	self->content = buffer_new();
	if (!self->content) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "content");
		goto fail;
	}
	
	return self;

fail:
	buffer_delete(self->buffer);
	buffer_delete(self->content);
	mem_free(self);
	return NULL;
}

void
response_clear(Response* self) {
	self->status = RESPONSE_INIT_STATUS;
	buffer_clear(self->buffer);
	buffer_clear(self->content);
}

void
response_set(Response* self, int status) {
	// Init
	response_clear(self);

	// Update status
	self->status = status;

	// Make buffer
	switch (status) {
	case 404: {
		buffer_append_string(self->buffer, "HTTP/1.1 404 Not Found\r\n");
		buffer_append_string(self->content, "<html><h1>404 Not Found</h1></html>\n");
	} break;
	case 405: {
		buffer_append_string(self->buffer, "HTTP/1.1 405 Method Not Allowed\r\n");
		buffer_append_string(self->content, "<html><h1>405 Method Not Allowed</h1></html>\n");
	} break;
	default: {
		buffer_append_string(self->buffer, "HTTP/1.1 500 Internal Server Error\r\n");
		buffer_append_string(self->content, "<html><h1>500 Internal Server Error. Unknown status.</h1></html>\n");
	} break;
	}

	buffer_append_string(self->buffer, "Server: ");
	buffer_append_string(self->buffer, SERVER_NAME);
	buffer_append_string(self->buffer, "\r\n");

	char contentlen[100/* TODO */];
	snprintf(contentlen, sizeof contentlen, "Content-Length: %d\r\n", (int) buffer_length(self->content));
	buffer_append_string(self->buffer, contentlen);
	buffer_append_string(self->buffer, "\r\n");

	// Merge content to buffer
	buffer_append_other(self->buffer, self->content);
}

/*********
* store *
*********/

typedef struct Store Store;

struct Store {
	Socket* client;
	HttpHeader* header;
	Response* response;
	char buffer[SERVER_NRECV_BUFFER];
};

void
store_delete(Store* self) {
	if (self) {
		httpheader_delete(self->header);	
		socket_close(self->client);
		response_delete(self->response);
		free(self);
	}
}

Store*
store_new(void) {
	Store* self = (Store*) mem_calloc(1, sizeof(Store));
	if (!self) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "store");
		return NULL;
	}

	self->header = httpheader_new();
	if (!self->header) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "HttpHeader");
		goto fail;
	}

	self->response = response_new();
	if (!self->response) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "Response");
		goto fail;
	}

	return self;

fail:
	httpheader_delete(self->header);
	response_delete(self->response);
	free(self);
	return NULL;
}

void
store_move_client(Store* self, Socket* client) {
	self->client = client;
}

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

static char const*
store_recv(Store* self) {
	int nrecv = socket_recv_string(self->client, self->buffer, sizeof self->buffer);
	if (nrecv <= 0) {
		WARN("Failed to recv");
		return NULL;
	}

	thread_eprintf("Recv (%d bytes) " , nrecv);
	term_ceprintf(TC_CYAN, TC_BLACK, "\"%s\"\n" , self->buffer);

	return self->buffer;
}

static HttpHeader*
store_parse_request(Store* self, char const* buffer) {
	if (!httpheader_parse_request(self->header, buffer)) {
		caperr_printf(PROGNAME, CAPERR_PARSE, "request");
		return NULL;
	}

	char const* methname = httpheader_method_name(self->header);
	char const* methvalue = httpheader_method_value(self->header);
	thread_eprintf("Request ");
	term_ceprintf(TC_YELLOW, TC_BLACK, "%s ", methname);
	term_ceprintf(TC_CYAN, TC_BLACK, "\"%s\"\n", methvalue);

	return self->header;
}

static Response*
store_response_from_header(Store* self, HttpHeader const* header) {
	char const* methname = httpheader_method_name(header);
	response_clear(self->response);

	if (strcmp(methname, "GET") == 0) {
		response_set(self->response, 404);
	} else {
		response_set(self->response, 405);
	}

	return self->response;
}

Store*
store_send_response(Store* self, Response* response) {
	unsigned char const* buf = buffer_get_const(response->buffer);
	size_t buflen = buffer_length(response->buffer);

	thread_eprintf("Send... (%d bytes)\n", buflen);
	thread_eprintf("response buffer[%s]\n", buf);

	if (socket_send_bytes(self->client, buf, buflen) < 0) {
		thread_eprintf("Failed to send");
		return NULL;
	}

	thread_eprintf("Success to send (%d bytes)\n", buflen);

	return self;
}

static void*
thread_main(void* arg) {
	thread_eputsf("Created");
	Store* store = (Store*) arg;

	for (;;) {
		char const* buffer = store_recv(store);
		if (!buffer) {
			break;
		}

		thread_eprintf("store_parse_request\n");
		HttpHeader* header = store_parse_request(store, buffer);
		if (!header) {
			caperr_printf(PROGNAME, CAPERR_PARSE, "request");
			continue;
		}

		thread_eprintf("store_response_from_header\n");
		Response* response = store_response_from_header(store, header);
		if (!response) {
			caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "response");
			continue;
		}

		thread_eprintf("store_send_response\n");
		if (!store_send_response(store, response)) {
			caperr_printf(PROGNAME, CAPERR_WRITE, "response");
			continue;			
		}
	}

	store_delete(store);
	thread_ceprintf(TC_MAGENTA, TC_BLACK, "Done\n");
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
		"    ** CAUTION!! DO NOT PUBLISHED THIS SERVER ON INTERNET. **\n\n", hostport
	);
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
			caperr_printf(PROGNAME, CAPERR_OPEN, "accept client");
			continue;
		}
		
		// Create Store
		Store* store = store_new();
		if (!store) {
			caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "store");
			socket_close(client);
			continue;	
		}

		store_move_client(store, client);

		// Thread works
		pthread_t thread;

		if (pthread_create(&thread, NULL, thread_main, (void*) store) != 0) {
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
