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

static char const*
status_line_from_version(double version, int status) {
	if (version == 1.1) {
		switch (status) {
		default: return "HTTP/1.1 500 Unknown status\r\n"; break;
		case 100: return "HTTP/1.1 100 Continue\r\n"; break; //	継続
		case 101: return "HTTP/1.1 101 Switching Protocols\r\n"; break; //	プロトコル切替
		case 200: return "HTTP/1.1 200 OK\r\n"; break; //	成功
		case 201: return "HTTP/1.1 201 Created\r\n"; break; //	作成完了
		case 202: return "HTTP/1.1 202 Accepted\r\n"; break; //	受理
		case 203: return "HTTP/1.1 203 Non-Authoritative Information\r\n"; break; //	非公式な情報
		case 204: return "HTTP/1.1 204 No Content\r\n"; break; //	内容が空
		case 205: return "HTTP/1.1 205 Reset Content\r\n"; break; //	内容をリセット
		case 206: return "HTTP/1.1 206 Partial Content\r\n"; break; //	内容の一部
		case 300: return "HTTP/1.1 300 Multiple Choices\r\n"; break; //	複数の候補がある
		case 301: return "HTTP/1.1 301 Moved Permanently\r\n"; break; //	恒久的に移転
		case 302: return "HTTP/1.1 302 Found\r\n"; break; //	別の場所で見つけた
		case 303: return "HTTP/1.1 303 See Other\r\n"; break; //	別の場所を探せ
		case 304: return "HTTP/1.1 304 Not Modified\r\n"; break; //	変更なし
		case 305: return "HTTP/1.1 305 Use Proxy\r\n"; break; //	中継サーバを通せ
		case 307: return "HTTP/1.1 307 Temporary Redirect\r\n"; break; //	一時的な転送
		case 400: return "HTTP/1.1 400 Bad Request\r\n"; break; //	不正なリクエスト
		case 401: return "HTTP/1.1 401 Unauthorized\r\n"; break; //	未認証
		case 402: return "HTTP/1.1 402 Payment Required\r\n"; break; //	有料である
		case 403: return "HTTP/1.1 403 Forbidden\r\n"; break; //	アクセス権がない
		case 404: return "HTTP/1.1 404 Not Found\r\n"; break; //	存在しない
		case 405: return "HTTP/1.1 405 Method Not Allowed\r\n"; break; //	そのメソッドは不可
		case 406: return "HTTP/1.1 406 Not Acceptable\r\n"; break; //	受理不可
		case 407: return "HTTP/1.1 407 Proxy Authentication Required\r\n"; break; //	中継サーバの認証が必要
		case 408: return "HTTP/1.1 408 Request Time-out\r\n"; break; //	時間切れ
		case 409: return "HTTP/1.1 409 Conflict\r\n"; break; //	競合
		case 410: return "HTTP/1.1 410 Gone\r\n"; break; //	消滅した
		case 411: return "HTTP/1.1 411 Length Required\r\n"; break; //	長さを指定せよ
		case 412: return "HTTP/1.1 412 Precondition Failed\r\n"; break; //	前提条件が満たされていない
		case 413: return "HTTP/1.1 413 Request Entity Too Large\r\n"; break; //	リクエスト中のデータが大きすぎる
		case 414: return "HTTP/1.1 414 Request-URI Too Large\r\n"; break; //	URIが長すぎる
		case 415: return "HTTP/1.1 415 Unsupported Media Type\r\n"; break; //	そのメディアは使えない
		case 500: return "HTTP/1.1 500 Internal Server Error\r\n"; break; //	サーバ内部のエラー
		case 501: return "HTTP/1.1 501 Not Implemented\r\n"; break; //	その機能は実装されていない
		case 502: return "HTTP/1.1 502 Bad Gateway\r\n"; break; //	中継サーバのエラー
		case 503: return "HTTP/1.1 503 Service Unavailable\r\n"; break; //	サービス停止中
		case 504: return "HTTP/1.1 504 Gateway Time-out\r\n"; break; //	中継サーバの要求が時間切れ
		case 505: return "HTTP/1.1 505 HTTP Version not supported\r\n"; break; //	そのバージョンのHTTPは使えない
		}
	} else {
		caperr_printf(PROGNAME, CAPERR_INVALID, "Http version");
		return "500 Internal Server Error\r\n";
	}
}

Response*
response_merge_content_with_status(Response* self, int status) {
	// Make buffer
	self->status = status;
	buffer_append_string(self->buffer, status_line_from_version(1.1, status));

	buffer_append_string(self->buffer, "Server: ");
	buffer_append_string(self->buffer, SERVER_NAME);
	buffer_append_string(self->buffer, "\r\n");

	char tmp[100/* TODO */];
	snprintf(tmp, sizeof tmp, "Content-Length: %d\r\n", (int) buffer_length(self->content));
	buffer_append_string(self->buffer, tmp);

	buffer_append_string(self->buffer, "\r\n");

	// Merge content
	buffer_append_other(self->buffer, self->content);

	// Done
	return self;
}

Response*
response_init_from_status(Response* self, int status) {
	// Init
	response_clear(self);

	// Update status
	self->status = status;

	// Make buffer
	switch (status) {
	case 404: buffer_append_string(self->content, "<html><h1>404 Not Found</h1></html>\n"); break;
	case 405: buffer_append_string(self->content, "<html><h1>405 Method Not Allowed</h1></html>\n"); break;
	case 500: buffer_append_string(self->content, "<html><h1>500 Internal Server Error</h1></html>\n"); break;
	default: buffer_append_string(self->content, "<html><h1>500 Internal Server Error. Unknown status.</h1></html>\n"); break;
	}

	return response_merge_content_with_status(self, status);
}

Response*
response_init_from_file(Response* self, char const* fname) {
	// Clear
	response_clear(self);

	// Read from file
	FILE* fin = file_open(fname, "rb");
	if (!fin) {
		caperr_printf(PROGNAME, CAPERR_FOPEN, "%s", fname);
		return response_init_from_status(self, 404);
	}

	buffer_append_string(self->content, "<pre>\n");

	if (buffer_append_stream(self->content, fin) < 0) {
		caperr_printf(PROGNAME, CAPERR_READ, "stream by \"%s\"", fname);
		return response_init_from_status(self, 500);
	}

	buffer_append_string(self->content, "</pre>\n");

	if (file_close(fin) != 0) {
		caperr_printf(PROGNAME, CAPERR_FCLOSE, "%s", fname);
		return response_init_from_status(self, 500);
	}

	return response_merge_content_with_status(self, 200);
}

Response*
response_init_from_dir(Response* self, char const* dirname, char const* dirpath) {
	// Clear
	response_clear(self);

	// Read directory
	Directory* dir = dir_open(dirpath);
	if (!dir) {
		caperr_printf(PROGNAME, CAPERR_OPEN, "directory \"%s\"", dirpath);
		return response_init_from_status(self, 400);
	}

	buffer_append_string(self->content, "<h1>Index of ");
	buffer_append_string(self->content, dirname);
	buffer_append_string(self->content, "</h1>\n");

	buffer_append_string(self->content, "<ul>\n");

	for (DirectoryNode* node; (node = dir_read_node(dir)); ) {
		char const* name = dirnode_name(node);
		buffer_append_string(self->content, "<li><a href=\"");
		if (strcmp(dirname, "/") != 0) {
			buffer_append_string(self->content, dirname);
			buffer_append_string(self->content, "/");
		}
		buffer_append_string(self->content, name);
		buffer_append_string(self->content, "\">");
		buffer_append_string(self->content, name);
		buffer_append_string(self->content, "</a></li>\n");
	}

	buffer_append_string(self->content, "</ul>\n");

	if (dir_close(dir) != 0) {
		caperr_printf(PROGNAME, CAPERR_CLOSE, "directory \"%s\"", dirpath);
		return response_init_from_status(self, 500);
	}

	return response_merge_content_with_status(self, 200);
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
	term_ceprintf(TC_YELLOW, TC_DEFAULT, "Thread %d: ", thread_id()); \
	term_eputsf(__VA_ARGS__); \
}

#define thread_eprintf(...) { \
	term_ceprintf(TC_YELLOW, TC_DEFAULT, "Thread %d: ", thread_id()); \
	term_eprintf(__VA_ARGS__); \
}

#define thread_ceprintf(fg, bg, ...) { \
	term_ceprintf(TC_YELLOW, TC_DEFAULT, "Thread %d: ", thread_id()); \
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
	term_ceprintf(TC_CYAN, TC_DEFAULT, "\"%s\"\n" , self->buffer);

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
	term_ceprintf(TC_YELLOW, TC_DEFAULT, "%s ", methname);
	term_ceprintf(TC_CYAN, TC_DEFAULT, "\"%s\"\n", methvalue);

	return self->header;
}

static Response*
store_response_from_get_method(Store* self, char const* value) {
	Config* config = config_instance();
	if (!config) {
		caperr_printf(PROGNAME, CAPERR_CONSTRUCT, "config");
		return response_init_from_status(self->response, 500);
	}

	char spath[FILE_NPATH];
	if (!config_path_with_home(config, spath, sizeof spath, value)) {
		caperr_printf(PROGNAME, CAPERR_MAKE, "path");
		return response_init_from_status(self->response, 500);
	}

	if (!file_is_exists(spath)) {
		return response_init_from_status(self->response, 404);
	}

	if (file_is_dir(spath)) {
		return response_init_from_dir(self->response, value, spath);
	}

	return response_init_from_file(self->response, spath);
}

static Response*
store_response_from_header(Store* self, HttpHeader const* header) {
	// Clear
	response_clear(self->response);

	// Get name and value from header
	char const* name = httpheader_method_name(header);
	char const* value = httpheader_method_value(header);

	// Switch by name
	if (strcmp(name, "GET") == 0) {
		return store_response_from_get_method(self, value);
	} else {
		response_init_from_status(self->response, 405);
	}

	return self->response;
}

Store*
store_send_response(Store* self, Response* response) {
	unsigned char const* buf = buffer_get_const(response->buffer);
	size_t buflen = buffer_length(response->buffer);

	thread_eprintf("Send... (%d bytes)\n", buflen);
	thread_eprintf("Send response buffer \"%s\"\n", buf);

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
	thread_ceprintf(TC_MAGENTA, TC_DEFAULT, "Done\n");
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
	term_ceprintf(TC_GREEN, TC_DEFAULT,
		"        _____    ___    _______       \n"
		"       /____/\\ /|___|\\ |\\______\\  \n"
		"      /     \\///     \\\\||    __ \\ \n"
		"     |    ==<//   |   \\||    ___|    \n"
		"      \\_____/ \\___^___/\\|___/      \n"
		"                                      \n"
	);
	term_ceprintf(TC_GREEN, TC_DEFAULT, "    CapSurver ");
	term_eprintf("running on ");
	term_ceprintf(TC_YELLOW, TC_DEFAULT, "%s\n", hostport);
	term_eprintf("    Run at ");
	term_ceprintf(TC_MAGENTA, TC_DEFAULT, "%s", ctime(&runtime));
	term_ceprintf(TC_RED, TC_DEFAULT,
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
