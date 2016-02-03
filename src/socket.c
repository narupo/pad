#include "socket.h"

# define WARN(...) { \
	fprintf(stderr, "Warn: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, ". %s.\n", strerror(errno)); \
	fflush(stderr); \
}

enum {
	SOCKET_NBUF = 512,
	SOCKET_NHOST = 128,
	SOCKET_NPORT = 32,
};

typedef enum {
	SocketModeNull,
	SocketModeTcpClient,
	SocketModeTcpServer,
} SocketMode;

struct Socket {
	char host[SOCKET_NHOST];
	char port[SOCKET_NPORT];
	SocketMode mode;
	int socket;
};

static char const SOCKET_DEFAULT_PORT[] = "8000";
static int is_init;

#if defined(_WIN32) || defined(_WIN64)
static WSADATA wsadata;
static void
cleanup(void) {
	WSACleanup();
}
#endif

static SocketMode
socket_string_to_type(char const* mode) {
	if (strcmp(mode, "tcp-server") == 0) {
		return SocketModeTcpServer;
	} else if (strcmp(mode, "tcp-client") == 0) {
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
		// if (close(self->socket) != 0) {
		//     Do not send  EOF to server
		// }
		free(self);
	}

	return 0;
}

Socket*
socket_open(char const* src, char const* mode) {
#if defined(_WIN32) || defined(_WIN64)
	if (!is_init) {
		if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
			WARN("Failed to start WSA. %d", WSAGetLastError());
			return NULL;
		}
		is_init = !is_init;
		atexit(cleanup);
	}
#endif

	Socket* self = (Socket*) calloc(1, sizeof(Socket));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	// Convert from string to number of mode
	self->mode = socket_string_to_type(mode);
	if (self->mode == SocketModeNull) {
		WARN("Invalid open mode \"%s\"", mode);
		free(self);
		return NULL;
	}

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
				free(self);
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

	// TCP Client
	
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
		WARN("Failed to getaddrinfo \"%s\"", src);
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
socket_recv_string(Socket* self, char* dst, size_t dstsz) {
	int ret = 0;

	ret = recv(self->socket, dst, dstsz, 0);
	if (ret < 0) {
		WARN("Failed to read from socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
	} else if (ret > 0) {
		dst[ret-1] = '\0';
	}

	return ret;
}

#if defined(TEST_SOCKET)
int
test_socket(int argc, char* argv[]) {
	if (argc < 2) {
		return 1;
	}

	Socket* socket = socket_open(argv[1], "tcp-client");
	if (!socket) {
		return 1;
	}

	socket_display(socket);

	socket_send_string(socket, "GET / \r\n\r\n");

	char buf[1024];
	for (;;) {
		if (socket_recv_string(socket, buf, sizeof buf) <= 0) {
			break;
		}
		printf("buf[%s]\n", buf);
	}


	socket_close(socket);
	return 0;
}

int main(int argc, char* argv[]) {
    return test_socket(argc, argv);
}
#endif
