#ifndef SOCKET_H
#define SOCKET_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
#else
# define _BSD_SOURCE
# include <netdb.h>
# include <sys/socket.h>
# include <netinet/in.h>
#endif

typedef struct Socket Socket;

#endif
