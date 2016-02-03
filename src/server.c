#include "server.h"

typedef struct Server Server;

struct Server {
	int argc;
	int optind;
	char** argv;

	bool opt_is_help;
};

static char const PROGNAME[] = "cap server";

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
	term_putsf("name[%s]", PROGNAME);
	term_flush();

	Socket* servsock = socket_open("127.0.0.1:1234", "tcp-server");
	if (!servsock) {
		return caperr_printf(PROGNAME, CAPERR_OPEN, "socket");
	}

	Socket* cliesock = socket_accept(servsock);
	if (!cliesock) {
		socket_close(servsock);
		return caperr_printf(PROGNAME, CAPERR_OPEN, "accept socket");
	}

	HttpHeader* header = httpheader_new();

	for (;;) {
		fprintf(stderr, "recv...\n");
		fflush(stderr);

		char buf[1024];
		int nrecv = socket_recv_string(cliesock, buf, sizeof buf);
		if (nrecv <= 0) {
			break;
		}

		term_putsf("buf[%s]", buf);
		term_flush();

		httpheader_parse_string(header, buf);
		httpheader_display(header);

		if (strcasecmp(httpheader_method_name(header), "GET") == 0) {
			fprintf(stderr, "send...\n");
			fflush(stderr);

			socket_send_string(cliesock,
				"HTTP/1.1 200 OK\r\n"
				"Server: CapServer (Prototype)\r\n"
				"Date: ^_^\r\n"
				"Content-Type: text/html; charset=utf-8\r\n"
				"Content-Length: 100\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				"<!DOCTYPE html>\n"
				"<html><head><title>test</title></head><body>\n"
				"Hello, World!\n"
				"日本語！\n"
				"</body></html>\n"
			);
		}

		if (strncmp(buf, "exit", 4) == 0) {
			break;
		}
	}

	httpheader_delete(header);
	socket_close(cliesock);
	socket_close(servsock);
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
