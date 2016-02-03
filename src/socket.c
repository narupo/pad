#include "socket.h"

# define WARN(...) { \
	fprintf(stderr, "Warn: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
}

enum {
	SOCKET_NBUF = 512,
	SOCKET_NHOST = 128,
	SOCKET_NPORT = 32,
};

typedef enum {
	SocketModeNull,
	SocketModeClient,
	SocketModeServer,
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
		return SocketModeServer;
	} else if (strcmp(mode, "tcp-client") == 0) {
		return SocketModeClient;
	} else {
		return SocketModeNull;
	}
}

void
socket_display(Socket const* self) {
	WARN("Socket host[%s] port[%s] mode[%d]\n", self->host, self->port, self->mode);
	fflush(stderr);
}

int
socket_close(Socket* self) {
	if (self) {
		if (close(self->socket) < 0) {
			WARN("Failed to close socket\n");
		}
		free(self);
	}

	return 0;
}

Socket*
socket_open(char const* src, char const* mode) {
#if defined(_WIN32) || defined(_WIN64)
	if (!is_init) {
		if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
			WARN("Failed to start WSA. %d\n", WSAGetLastError());
			return NULL;
		}
		is_init = !is_init;
		atexit(cleanup);
	}
#endif

	Socket* self = (Socket*) calloc(1, sizeof(Socket));
	if (!self) {
		WARN("Failed to allocate memory\n");
		return NULL;
	}

	// Convert from string to number of mode
	self->mode = socket_string_to_type(mode);
	if (self->mode == SocketModeNull) {
		WARN("Invalid open mode \"%s\"\n", mode);
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
				WARN("Invalid port number of \"%s\"\n", src);
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

	struct addrinfo* infores = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

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
			break; // Success to connect
		}

		close(self->socket);
	}

	if (!rp) {
		WARN("Could not connect to any address \"%s:%s\"\n", self->host, self->port);
		freeaddrinfo(infores);
		free(self);
	}

	freeaddrinfo(infores);

	return self;
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

	socket_close(socket);
	return 0;
}

int main(int argc, char* argv[]) {
    return test_socket(argc, argv);
}
#endif
