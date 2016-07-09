#include <stdio.h>
#include "socket.h"

int
main(int argc, char* argv[]) {
	struct cap_socket *sock = cap_sockopen("http://localhost:80", "tcp-client");

	cap_sockdisp(sock);
	cap_socksendstr(sock, "GET / HTTP1.1\r\n\r\n");

	char dst[5096];
	cap_sockrecvstr(sock, dst, sizeof dst);
	printf("%s\n", dst);

	cap_sockclose(sock);
	return 0;
}

