#include "socket.h"

#if defined(TEST_SOCKET)

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
#endif

#include <stdio.h>

int
main(int argc, char* argv[]) {
	WSADATA wsa;
	WSAStartup(2, &wsa);


	WSACleanup();
    return 0;
}
#endif

