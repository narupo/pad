/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "socket.h"

enum {
	SOCKET_NBUF = 512,
	SOCKET_NHOST = 128,
	SOCKET_NPORT = 32,
};

typedef enum {
	SOCKMODE_NULL,
	SOCKMODE_TCPCLIENT,
	SOCKMODE_TCPSERVER,
	SOCKMODE_ACCEPTCLIENT,
} cap_sockmode_t;

static const char SOCKET_DEFAULT_PORT[] = "8000";

struct cap_socket {
	char host[SOCKET_NHOST];
	char port[SOCKET_NPORT];
	int socket;
	cap_sockmode_t mode;
};

/*************
* socket log *
*************/

void
socklog(const char *fmt, ...) {
	size_t fmtlen = strlen(fmt);
	va_list args;
	va_start(args, fmt);

	fflush(stdout);
	fprintf(stderr, "cap socket: ");

	if (isalpha(fmt[0])) {
		fprintf(stderr, "%c", toupper(fmt[0]));
		vfprintf(stderr, fmt+1, args);
	} else {
		vfprintf(stderr, fmt, args);
	}

	if (fmtlen && fmt[fmtlen-1] != '.') {
		fprintf(stderr, ". ");
	}

	if (errno != 0) {
		fprintf(stderr, "%s.", strerror(errno));
	}

	fprintf(stderr, "\n");

	va_end(args);
	fflush(stderr);
	exit(EXIT_FAILURE);
}

/*******************************
* socket WSA family of Windows *
*******************************/

#if defined(_CAP_WINDOWS)
static pthread_once_t wsasockonce = PTHREAD_ONCE_INIT;
static WSADATA wsadata;

static void
wsasockdestroy(void) {
	WSACleanup();
}

static void
wsasockinit(void) {
	if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
		socklog("failed to start WSA. %d", WSAGetLastError());
	} else {
		atexit(wsasockdestroy);
	}
}
#endif /* _CAP_WINDOWS */

/*******************
* socket functions *
*******************/

static cap_sockmode_t
cap_sockstr2mode(const char *mode) {
	if (strcasecmp(mode, "tcp-server") == 0) {
		return SOCKMODE_TCPSERVER;
	} else if (strcasecmp(mode, "tcp-client") == 0) {
		return SOCKMODE_TCPCLIENT;
	} else {
		return SOCKMODE_NULL;
	}
}

static const char *
cap_sockmode2str(cap_sockmode_t mode) {
	switch (mode) {
	case SOCKMODE_NULL: return "null"; break;
	case SOCKMODE_TCPCLIENT: return "tcp-client"; break;
	case SOCKMODE_TCPSERVER: return "tcp-server"; break;
	case SOCKMODE_ACCEPTCLIENT: return "tcp-accept-client"; break;
	default: return "unknown"; break;
	}
}

void
cap_sockshow(const struct cap_socket *self) {
	socklog("socket host[%s] port[%s] mode[%s] socket[%d]\n"
		, self->host, self->port, cap_sockmode2str(self->mode), self->socket);
	fflush(stderr);
}

int
cap_sockclose(struct cap_socket *self) {
	if (self) {
		// if (close(self->socket) < 0) {
		// 	socklog("failed to close socket [%d] by \"%s:%s\""
		// 		, self->socket, self->host, self->port);
		// }
		free(self);
	}

	return 0;
}

static struct cap_socket *
cap_sockinittcpserver(struct cap_socket *self) {
	struct addrinfo *infores = NULL;
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
		socklog("failed to getaddrinfo \"%s:%s\"", self->host, self->port);
		free(self);
		return NULL;
	}

	// Get listen socket
	struct addrinfo *rp;

	for (rp = infores; rp; rp = rp->ai_next) {
		self->socket = socket(infores->ai_family, infores->ai_socktype, infores->ai_protocol);
		if (self->socket < 0) {
			continue;
		}

		int optval;
		if (setsockopt(self->socket, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, sizeof(optval)) == -1) {
			socklog("failed to setsockopt \"%s:%s\"", self->host, self->port);
			freeaddrinfo(infores);
			free(self);
			return NULL;
		}

		if (bind(self->socket, rp->ai_addr, (int) rp->ai_addrlen) == 0) {
			break; // success to bind
		}

		close(self->socket);
	}

	freeaddrinfo(infores);

	if (!rp) {
		socklog("failed to find listen socket");
		free(self);
		return NULL;
	}

	if (listen(self->socket, SOMAXCONN) < 0) {
		socklog("failed to listen \"%s:%s\"", self->host, self->port);
		free(self);
		return NULL;
	}

	return self;
}

static struct cap_socket *
cap_sockinittcpclient(struct cap_socket *self) {
	struct addrinfo *infores = NULL;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = IPPROTO_TCP;

	if (getaddrinfo(self->host, self->port, &hints, &infores) != 0) {
		socklog("failed to getaddrinfo \"%s:%s\"", self->host, self->port);
		free(self);
		return NULL;
	}

	// Find can connect structure
	struct addrinfo *rp = NULL;

	for (rp = infores; rp; rp = rp->ai_next) {
		self->socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (self->socket == -1) {
			continue;
		}		

		if (connect(self->socket, rp->ai_addr, rp->ai_addrlen) != -1) {
			socklog("success to connect [%d]", self->socket);
			break; // success to connect
		}

		if (close(self->socket) < 0) {
			socklog("failed to close socket [%d]", self->socket);
		}
	}

	freeaddrinfo(infores);

	if (!rp) {
		socklog("could not connect to any address \"%s:%s\"\n", self->host, self->port);
		free(self);
		return NULL;
	}

	return self;
}

static struct cap_socket *
cap_sockparseopensrc(struct cap_socket *self, const char *src) {
	char *dst = self->host;
	int ndst = sizeof(self->host)-1;
	int di = 0;
	int m = 0;

	for (const char *sp = src; *sp; ++sp) {
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
				socklog("invalid port number of \"%s\"", src);
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

struct cap_socket *
cap_sockopen(const char *src, const char *mode) {
#if defined(_CAP_WINDOWS)
	if (pthread_once(&wsasockonce, wsasockinit) != 0) {
		socklog("failed to pthread once");
		return NULL;
	}
#endif

	struct cap_socket *self = calloc(1, sizeof(struct cap_socket));
	if (!self) {
		socklog("failed to allocate memory");
		return NULL;
	}

	// Convert from string to number of mode
	self->mode = cap_sockstr2mode(mode);
	if (self->mode == SOCKMODE_NULL) {
		socklog("invalid open mode \"%s\"", mode);
		free(self);
		return NULL;
	}

	// Parse source for host and port
	if (!cap_sockparseopensrc(self, src)) {
		socklog("failed to parse of \"%s\"", src);
		free(self);
		return NULL;
	}

	// Init by mode
	switch (self->mode) {
	case SOCKMODE_TCPSERVER: return cap_sockinittcpserver(self); break;
	case SOCKMODE_TCPCLIENT: return cap_sockinittcpclient(self); break;
	default: break;
	}

	// Invalid open mode
	socklog("invalid open mode \"%s:%s\"", self->host, self->port);
	free(self);
	return NULL;
}

const char *
cap_sockhost(const struct cap_socket *self) {
	return self->host;
}

const char *
cap_sockport(const struct cap_socket *self) {
	return self->port;
}

struct cap_socket *
cap_sockaccept(const struct cap_socket *self) {
	if (self->mode != SOCKMODE_TCPSERVER) {
		socklog("invalid mode on accept \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	int cliefd = accept(self->socket, NULL, NULL);
	if (cliefd < 0) {
		socklog("failed to accept \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	struct cap_socket *client = calloc(1, sizeof(struct cap_socket));
	if (!client) {
		socklog("failed to construct socket");
		return NULL;
	}

	client->socket = cliefd;
	client->mode = SOCKMODE_ACCEPTCLIENT;

	return client;
}

int
cap_sockrecvstr(struct cap_socket *self, char *dst, size_t dstsz) {
	if (!dst || dstsz < 1) {
		socklog("invalid arguments");
		return -1;
	}

	int ret = recv(self->socket, dst, dstsz-1, 0);
	if (ret < 0) {
		socklog("failed to read from socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
		*dst = '\0';
	} else if (ret > 0) {
		dst[ret] = '\0';
	}

	return ret;
}

int
cap_socksendstr(struct cap_socket *self, const char *str) {
	int ret = 0;
	size_t len = strlen(str);

	ret = send(self->socket, str, len, 0);
	if (ret < 0) {
		socklog("failed to write to socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
	}

	return ret;
}

int 
cap_socksend(struct cap_socket *self, const unsigned char *bytes, size_t size) {
	int ret = 0;

#if defined(_CAP_WINDOWS)
	ret = send(self->socket, (const char *)bytes, size, 0);
#else
	ret = send(self->socket, bytes, size, 0);
#endif
	if (ret < 0) {
		socklog("failed to write to socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
	}

	return ret;
}

#if defined(_TEST_SOCKET)
int
testtcpserv(int argc, char *argv[]) {
	if (argc < 2) {
		return 1;
	}

	struct cap_socket *server = cap_sockopen(argv[1], "tcp-server");
	if (!server) {
		return 1;
	}

	cap_sockshow(server);

	// Single I/O
	socklog("accept...\n");
	fflush(stderr);
	struct cap_socket *client = cap_sockaccept(server);

	for (;;) {

		socklog("recv...\n");
		fflush(stderr);

		char buf[1024];
		cap_sockrecvstr(client, buf, sizeof buf);
		printf("buf[%s]\n", buf);
		fflush(stdout);

		if (strncmp(buf, "exit", 4) == 0) {
			break;
		}
	}

	cap_sockclose(client);
	cap_sockclose(server);
	return 0;
}

int
testtcpclient(int argc, char *argv[]) {
	if (argc < 2) {
		return 1;
	}

	struct cap_socket *socket = cap_sockopen(argv[1], "tcp-client");
	if (!socket) {
		return 1;
	}

	cap_sockshow(socket);

	cap_socksendstr(socket,
		"GET / HTTP/1.1\r\n"
		"Host: smile.com\r\n"
		"\r\n\r\n"
	);

	char buf[1024];
	cap_sockrecvstr(socket, buf, sizeof buf);
	
	printf("buf[%s]\n", buf);
	fflush(stdout);
	fflush(stderr);

	cap_sockclose(socket);
	return 0;
}

int main(int argc, char *argv[]) {
    // return testtcpclient(argc, argv);
    return testtcpserv(argc, argv);
}
#endif

