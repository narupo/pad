#include "server.h"

typedef struct Server Server;

struct Server {
	int argc;
	int optind;
	char** argv;

	bool opt_is_help;
};

static char const PROGNAME[] = "cap server";
static char const DEFAULT_HOSTPORT[] = "127.0.0.1:1234";

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
thread_method_get(HttpHeader const* header, Socket* client) {
	Config* config = config_instance();
	char const* methval = httpheader_method_value(header);
	char path[FILE_NPATH];

	config_path_with_home(config, path, sizeof path, methval);
	term_eputsf("Thread %d path[%s]", pthread_self(), path);
	
	if (!file_is_exists(path)) {
		socket_send_string(client,
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Length: 0\r\n"
			"\r\n"
		);
		return;
	}

	if (file_is_dir(path)) {
		socket_send_string(client,
			"HTTP/1.1 403 Permission denied\r\n"
			"Content-Length: 0\r\n"
			"\r\n"
		);
		return;
	}

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
	String* sendbuf = str_new();
	char tmp[100];
	snprintf(tmp, sizeof tmp, "Content-Length: %d\r\n", str_length(content));

	str_append_string(sendbuf,
		"HTTP/1.1 200 OK\r\n"
		"Server: CapServer\r\n"
	);
	str_append_string(sendbuf, tmp);
	str_append_string(sendbuf, "\r\n");
	str_append_other(sendbuf, content);

	// Send
	socket_send_string(client, str_get_const(sendbuf));

	// Done
	str_delete(sendbuf);
	str_delete(content);
}

static void*
thread_main(void* arg) {
	Socket* client = (Socket*) arg;
	char buf[512];

	HttpHeader* header = httpheader_new();
	if (!header) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "HttpHeader");
	}

	term_eputsf("Thread %d running...", pthread_self());

	for (;;) {
		int nrecv = socket_recv_string(client, buf, sizeof buf);
		if (nrecv < 0) {
			WARN("Failed to recv");
			break;
		}

		httpheader_parse_request(header, buf);
		char const* methname = httpheader_method_name(header);

		if (strcmp(methname, "GET") == 0) {
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

static int
server_run(Server* self) {
	char const* hostport = DEFAULT_HOSTPORT;
	if (self->argc > self->optind) {
		hostport = self->argv[self->optind];
	}

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
        "\t%s [option]... [arguments]\n"
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
