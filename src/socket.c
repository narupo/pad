#include "socket.h"

enum {
	SOCKET_NBUF = 512,
	SOCKET_NHOST = 128,
	SOCKET_NPORT = 32,
};

typedef enum {
	SocketModeNull,
	SocketModeTcpClient,
	SocketModeTcpServer,

	SocketModeAcceptClient,

} SocketMode;

static char const SOCKET_DEFAULT_PORT[] = "8000";

struct Socket {
	char host[SOCKET_NHOST];
	char port[SOCKET_NPORT];
	SocketMode mode;
	int socket;
};

/*******************************
* socket WSA family of Windows *
*******************************/

#if defined(_CAP_WINDOWS)
static pthread_once_t socket_wsa_once = PTHREAD_ONCE_INIT;
static WSADATA wsadata;

static void
socket_wsa_destroy(void) {
	WSACleanup();
}

static void
socket_wsa_init(void) {
	if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
		WARN("Failed to start WSA. %d", WSAGetLastError());
	} else {
		atexit(socket_wsa_destroy);
	}
}
#endif

/*******************
* socket functions *
*******************/

static SocketMode
socket_string_to_mode(char const* mode) {
	if (strcasecmp(mode, "tcp-server") == 0) {
		return SocketModeTcpServer;
	} else if (strcasecmp(mode, "tcp-client") == 0) {
		return SocketModeTcpClient;
	} else {
		return SocketModeNull;
	}
}

void
socket_display(Socket const* self) {
	fprintf(stderr, "Socket host[%s] port[%s] mode[%d] socket[%d]\n"
		, self->host, self->port, self->mode, self->socket);
	fflush(stderr);
}

int
socket_close(Socket* self) {
	if (self) {
		// if (close(self->socket) < 0) {
		// 	WARN("Failed to close socket [%d] by \"%s:%s\""
		// 		, self->socket, self->host, self->port);
		// }
		free(self);
	}

	return 0;
}

static Socket*
socket_init_tcp_server(Socket* self) {
	struct addrinfo* infores = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, self->port, &hints, &infores) != 0) {
		WARN("Failed to getaddrinfo \"%s:%s\"", self->host, self->port);
		free(self);
		return NULL;
	}

	// Get listen socket
	struct addrinfo* rp;

	for (rp = infores; rp; rp = rp->ai_next) {
		self->socket = socket(infores->ai_family, infores->ai_socktype, infores->ai_protocol);
		if (self->socket < 0) {
			continue;
		}

		int optval;
		if (setsockopt(self->socket, SOL_SOCKET, SO_REUSEADDR, (void*) &optval, sizeof(optval)) == -1) {
			WARN("Failed to setsockopt \"%s:%s\"", self->host, self->port);
			freeaddrinfo(infores);
			free(self);
			return NULL;
		}

		if (bind(self->socket, rp->ai_addr, (int) rp->ai_addrlen) == 0) {
			break; // Success to bind
		}

		close(self->socket);
	}

	freeaddrinfo(infores);

	if (!rp) {
		WARN("Failed to find listen socket");
		free(self);
		return NULL;
	}

	if (listen(self->socket, SOMAXCONN) < 0) {
		WARN("Failed to listen \"%s:%s\"", self->host, self->port);
		free(self);
		return NULL;
	}

	return self;
}

static Socket*
socket_init_tcp_client(Socket* self) {
	struct addrinfo* infores = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = IPPROTO_TCP;

	if (getaddrinfo(self->host, self->port, &hints, &infores) != 0) {
		WARN("Failed to getaddrinfo \"%s:%s\"", self->host, self->port);
		free(self);
		return NULL;
	}

	// Find can connect structure
	struct addrinfo* rp = NULL;

	for (rp = infores; rp; rp = rp->ai_next) {
		self->socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (self->socket == -1) {
			continue;
		}		

		if (connect(self->socket, rp->ai_addr, rp->ai_addrlen) != -1) {
			WARN("Success to connect [%d]", self->socket);
			break; // Success to connect
		}

		if (close(self->socket) < 0) {
			WARN("Failed to close socket [%d]", self->socket);
		}
	}

	freeaddrinfo(infores);

	if (!rp) {
		WARN("Could not connect to any address \"%s:%s\"\n", self->host, self->port);
		free(self);
		return NULL;
	}

	return self;
}

static Socket*
socket_parse_open_source(Socket* self, char const* src) {
	int m = 0;
	char* dst = self->host;
	int ndst = sizeof(self->host)-1;
	int di = 0;

	for (char const* sp = src; *sp; ++sp) {
		switch (m) {
		case 0:
			if (*sp == ':') {
				m = 1;
				di = 0;
				dst = self->port;
				ndst = sizeof(self->port)-1;
			} else {
				if (di < ndst) {
					dst[di++] = *sp;
					dst[di] = '\0';
				}
			}
			break;
		case 1:
			if (!isdigit((int) *sp)) {
				WARN("Invalid port number of \"%s\"", src);
				return NULL;
			}
		
			if (di < ndst) {
				dst[di++] = *sp;
				dst[di] = '\0';
			}
		
			break;
		}
	}

	if (self->port[0] == '\0') {
		snprintf(self->port, sizeof self->port, "%s", SOCKET_DEFAULT_PORT);
	}

	return self;
}

Socket*
socket_open(char const* src, char const* mode) {
#if defined(_CAP_WINDOWS)
	if (pthread_once(&socket_wsa_once, socket_wsa_init) != 0) {
		WARN("Failed to pthread once");
		return NULL;
	}
#endif

	Socket* self = (Socket*) calloc(1, sizeof(Socket));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	// Convert from string to number of mode
	self->mode = socket_string_to_mode(mode);
	if (self->mode == SocketModeNull) {
		WARN("Invalid open mode \"%s\"", mode);
		free(self);
		return NULL;
	}

	// Parse source for host and port
	if (!socket_parse_open_source(self, src)) {
		WARN("Failed to parse of \"%s\"", src);
		free(self);
		return NULL;
	}

	// Init by mode
	switch (self->mode) {
	case SocketModeTcpServer: return socket_init_tcp_server(self); break;
	case SocketModeTcpClient: return socket_init_tcp_client(self); break;
	default: break;
	}

	// Invalid open mode
	WARN("Invalid open mode \"%s:%s\"", self->host, self->port);
	free(self);
	return NULL;
}

char const*
socket_host(Socket const* self) {
	return self->host;
}

char const*
socket_port(Socket const* self) {
	return self->port;
}

Socket*
socket_accept(Socket const* self) {
	if (self->mode != SocketModeTcpServer) {
		WARN("Invalid mode on accept \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	int cliefd = accept(self->socket, NULL, NULL);
	if (cliefd < 0) {
		WARN("Failed to accept \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	Socket* client = (Socket*) calloc(1, sizeof(Socket));
	if (!client) {
		WARN("Failed to construct socket");
		return NULL;
	}

	client->socket = cliefd;
	client->mode = SocketModeAcceptClient;

	return client;
}

int
socket_recv_string(Socket* self, char* dst, size_t dstsz) {
	int ret = 0;

	ret = recv(self->socket, dst, dstsz-1, 0);
	if (ret < 0) {
		WARN("Failed to read from socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
		*dst = '\0';
	} else if (ret > 0) {
		dst[ret] = '\0';
	}

	return ret;
}

int
socket_send_string(Socket* self, char const* str) {
	int ret = 0;
	size_t len = strlen(str);

	ret = send(self->socket, str, len, 0);
	if (ret < 0) {
		WARN("Failed to write to socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
	}

	return ret;
}

int 
socket_send_bytes(Socket* self, unsigned char const* bytes, size_t size) {
	int ret = 0;

#if defined(_CAP_WINDOWS)
	ret = send(self->socket, (char const*) bytes, size, 0);
#else
	ret = send(self->socket, bytes, size, 0);
#endif
	if (ret < 0) {
		WARN("Failed to write to socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
	}

	return ret;
}

#if defined(TEST_SOCKET)
int
test_tcp_server(int argc, char* argv[]) {
	if (argc < 2) {
		return 1;
	}

	Socket* server = socket_open(argv[1], "tcp-server");
	if (!server) {
		return 1;
	}

	socket_display(server);

	// Single I/O
	fprintf(stderr, "accept...\n");
	fflush(stderr);
	Socket* client = socket_accept(server);

	for (;;) {

		fprintf(stderr, "recv...\n");
		fflush(stderr);

		char buf[1024];
		socket_recv_string(client, buf, sizeof buf);
		printf("buf[%s]\n", buf);
		fflush(stdout);

		if (strncmp(buf, "exit", 4) == 0) {
			break;
		}
	}

	socket_close(client);
	socket_close(server);
	return 0;
}

int
test_tcp_client(int argc, char* argv[]) {
	if (argc < 2) {
		return 1;
	}

	Socket* socket = socket_open(argv[1], "tcp-client");
	if (!socket) {
		return 1;
	}

	socket_display(socket);

	socket_send_string(socket,
		"GET / HTTP/1.1\r\n"
		"Host: smile.com\r\n"
		"\r\n\r\n"
	);

	char buf[1024];
	socket_recv_string(socket, buf, sizeof buf);
	
	printf("buf[%s]\n", buf);
	fflush(stdout);
	fflush(stderr);

	socket_close(socket);
	return 0;
}

int main(int argc, char* argv[]) {
    // return test_tcp_client(argc, argv);
    return test_tcp_server(argc, argv);
}
#endif
