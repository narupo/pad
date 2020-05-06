/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "socket.h"

enum {
	SOCK_NBUF = 512,
	SOCK_NHOST = 128,
	SOCK_NPORT = 32,
    SOCK_ERR_SIZE = 1024,
};

typedef enum {
	SOCK_MODE_NULL,
	SOCK_MODE_TCPCLIENT,
	SOCK_MODE_TCPSERVER,
	SOCK_MODE_ACCEPTCLIENT,
} sock_mode_t;

static const char *SOCK_DEFAULT_PORT = "8000";

struct socket {
	char host[SOCK_NHOST];
	char port[SOCK_NPORT];
	int32_t socket;
	sock_mode_t mode;
    char error[SOCK_ERR_SIZE];
};

/*******************************
* socket WSA family of Windows *
*******************************/

#if defined(_SOCK_WINDOWS)
static pthread_once_t wsa_sock_once = PTHREAD_ONCE_INIT;
static WSADATA wsa_data;

static void
wsa_sock_destroy(void) {
	WSACleanup();
}

static void
wsa_sock_init(void) {
	if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0) {
		fprintf(stderr, "failed to start WSA. %d", WSAGetLastError());
	} else {
		atexit(wsa_sock_destroy);
	}
}
#endif /* _SOCK_WINDOWS */

/*******************
* socket functions *
*******************/

static sock_mode_t
sock_str_to_mode(const char *mode) {
	if (strcasecmp(mode, "tcp-server") == 0) {
		return SOCK_MODE_TCPSERVER;
	} else if (strcasecmp(mode, "tcp-client") == 0) {
		return SOCK_MODE_TCPCLIENT;
	} else {
		return SOCK_MODE_NULL;
	}
}

static const char *
sock_mode_to_uint8(sock_mode_t mode) {
	switch (mode) {
	case SOCK_MODE_NULL: return "null"; break;
	case SOCK_MODE_TCPCLIENT: return "tcp-client"; break;
	case SOCK_MODE_TCPSERVER: return "tcp-server"; break;
	case SOCK_MODE_ACCEPTCLIENT: return "tcp-accept-client"; break;
	default: return "unknown"; break;
	}
}

void
sock_dump(const socket_t *self, FILE *fout) {
	fprintf(
        fout,
        "socket host[%s] port[%s] mode[%s] socket[%d]\n",
        self->host,
        self->port,
        sock_mode_to_uint8(self->mode),
        self->socket
    );
}

int32_t
sock_close(socket_t *self) {
	if (self) {
		// if (close(self->socket) < 0) {
		// 	socklog("failed to close socket [%d] by \"%s:%s\""
		// 		, self->socket, self->host, self->port);
		// }
		free(self);
	}

	return 0;
}

void
sock_set_error(socket_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(self->error, sizeof self->error, fmt, ap);

    va_end(ap);
}

const char *
sock_getc_error(const socket_t *self) {
    if (!self) {
        return "socket is null";
    }

    return self->error;
}

bool
sock_has_error(const socket_t *self) {
    if (!self || self->error[0] != '\0') {
        return true;
    }

    return false;
}

static socket_t *
init_tcp_server(socket_t *self) {
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
		sock_set_error(self, "failed to getaddrinfo \"%s:%s\"", self->host, self->port);
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
			sock_set_error(self, "failed to setsockopt \"%s:%s\"", self->host, self->port);
			freeaddrinfo(infores);
			return NULL;
		}

		if (bind(self->socket, rp->ai_addr, (int) rp->ai_addrlen) == 0) {
			break; // success to bind
		}

		close(self->socket);
	}

	freeaddrinfo(infores);

	if (!rp) {
		sock_set_error(self, "failed to find listen socket");
		return NULL;
	}

	if (listen(self->socket, SOMAXCONN) < 0) {
		sock_set_error(self, "failed to listen \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	return self;
}

static socket_t *
init_tcp_client(socket_t *self) {
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
		sock_set_error(self, "failed to getaddrinfo \"%s:%s\"", self->host, self->port);
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
			// printf("success to connect [%d]", self->socket);
			break; // success to connect
		}

		if (close(self->socket) < 0) {
			sock_set_error(self, "failed to close socket [%d]", self->socket);
		}
	}

	freeaddrinfo(infores);

	if (!rp) {
		sock_set_error(
            self,
            "could not connect to any address \"%s:%s\"\n",
            self->host,
            self->port
        );
		return NULL;
	}

	return self;
}

static socket_t *
parse_open_src(socket_t *self, const char *src) {
	if (!self || !src) {
		sock_set_error(self, "invalid arguments");
		return NULL;
	}

	char *dst = self->host;
	int32_t ndst = sizeof(self->host)-1;
	int32_t di = 0;
	int32_t m = 0;

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
			if (!isdigit(*sp)) {
				sock_set_error(self, "invalid port number of \"%s\"", src);
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
		snprintf(self->port, sizeof self->port, "%s", SOCK_DEFAULT_PORT);
	}

	return self;
}

socket_t *
sock_open(const char *src, const char *mode) {
	socket_t *self = calloc(1, sizeof(socket_t));
	if (!self) {
        perror("calloc");
		return NULL;
	}

	if (!src || !mode) {
		sock_set_error(self, "invalid arguments");
		return NULL;
	}

#if defined(_CAP_WINDOWS)
	if (pthread_once(&wsa_sock_once, wsa_sock_init) != 0) {
		sock_set_error(self, "failed to pthread once");
		return NULL;
	}
#endif

	// convert from string to number of mode
	self->mode = sock_str_to_mode(mode);
	if (self->mode == SOCK_MODE_NULL) {
		sock_set_error(self, "invalid open mode \"%s\"", mode);
		return NULL;
	}

	// parse source for host and port
	if (!parse_open_src(self, src)) {
		sock_set_error(self, "failed to parse of \"%s\"", src);
		return NULL;
	}

	// init by mode
	switch (self->mode) {
    default:
        break;
	case SOCK_MODE_TCPSERVER:
        return init_tcp_server(self);
        break;
	case SOCK_MODE_TCPCLIENT:
        return init_tcp_client(self);
        break;
	}

	// invalid open mode
	sock_set_error(self, "invalid open mode \"%s:%s\"", self->host, self->port);
	return NULL;
}

const char *
sock_getc_host(const socket_t *self) {
	if (!self) {
		return NULL;
	}

	return self->host;
}

const char *
sock_getc_port(const socket_t *self) {
	if (!self) {
		return NULL;
	}

	return self->port;
}

socket_t *
sock_accept(socket_t *self) {
	if (!self) {
		return NULL;
	}

	if (self->mode != SOCK_MODE_TCPSERVER) {
		sock_set_error(self, "invalid mode on accept \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	int32_t cliefd = accept(self->socket, NULL, NULL);
	if (cliefd < 0) {
		sock_set_error(self, "failed to accept \"%s:%s\"", self->host, self->port);
		return NULL;
	}

	socket_t *client = calloc(1, sizeof(socket_t));
	if (!client) {
		sock_set_error(self, "failed to construct socket");
		return NULL;
	}

	client->socket = cliefd;
	client->mode = SOCK_MODE_ACCEPTCLIENT;

	return client;
}

int32_t
sock_recv_str(socket_t *self, char *dst, int32_t dstsz) {
    if (!self) {
        return -1;
    }

	if (!dst || dstsz < 1) {
		sock_set_error(self, "invalid arguments");
		return -1;
	}

	int32_t ret = recv(self->socket, dst, dstsz-1, 0);
	if (ret < 0) {
		sock_set_error(self, "failed to read from socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
		*dst = '\0';
	} else if (ret > 0) {
		dst[ret] = '\0';
	}

	return ret;
}

int32_t
sock_send_str(socket_t *self, const char *str) {
    if (!self) {
        return -1;
    }

	if (!str) {
		sock_set_error(self, "invalid arguments");
		return -1;
	}

	int32_t ret = 0;
	int32_t len = strlen(str);

	ret = send(self->socket, str, len, 0);
	if (ret < 0) {
		sock_set_error(self, "failed to write to socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
        return -1;
	}

	return ret;
}

int32_t
sock_send(socket_t *self, const char *bytes, int32_t size) {
    if (!self) {
        return -1;
    }

	if (size <= 0) {
		sock_set_error(self, "invalid arguments");
		return -1;
	}

	int32_t ret = 0;

#if defined(_CAP_WINDOWS)
	ret = send(self->socket, bytes, size, 0);
#else
	ret = send(self->socket, bytes, size, 0);
#endif
	if (ret < 0) {
		sock_set_error(self, "failed to write to socket [%d] by \"%s:%s\""
			, self->socket, self->host, self->port);
	}

	return ret;
}
