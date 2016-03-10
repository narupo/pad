#ifndef SOCKET_H
#define SOCKET_H

#include "define.h"
#include "util.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#if defined(_CAP_WINDOWS)
#  include "windows.h"
#else
#  undef _BSD_SOURCE
#  define _BSD_SOURCE 1 /* For netdb.h in cap/socket.h */
#  undef __USE_POSIX
#  define __USE_POSIX 1 /* For netdb.h in cap/socket.h */
#  include <netdb.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

typedef struct Socket Socket;

/**
 * Display parameters of socket
 *
 * @param[in] self 
 */
void 
socket_display(Socket const* self);

/**
 * Close socket
 * If self is NULL then don't anything
 *
 * @param[in] self 
 *
 * @return success to number of zero
 * @return failed to under number of zero
 */
int 
socket_close(Socket* self);

/**
 * Open socket by source and mode like a fopen(3)
 * Open mode are:
 *
 * 		tcp-client
 * 		tcp-server
 *
 * @param[in] src  format of "host:port" of C string
 * @param[in] mode open mode
 *
 * @return pointer to dynamic allocate memory of Socket
 */
Socket* 
socket_open(char const* src, char const* mode);

/**
 * Get host of C string by socket
 *
 * @param[in] self
 *
 * @return pointer to memory of C string
 */
char const*
socket_host(Socket const* self);

/**
 * Get port of C string by socket
 *
 * @param[in] self
 *
 * @return pointer to memory of C string
 */
char const*
socket_port(Socket const* self);

/**
 * Wrapper of accept(2)
 * Get new socket by self socket 
 *
 * @param[in] self 
 *
 * @return pointer to dynamic allocate memory of Socket of client
 */
Socket* 
socket_accept(Socket const* self);

/**
 * Wrapper of recv(2)
 * Recv string from socket
 *
 * @param[in] self  
 * @param[in] dst   pointer to memory of destination buffer
 * @param[in] dstsz number of size of destination buffer
 *
 * @return success to number of recv size
 * @return failed to number of under of zero
 */
int
socket_recv_string(Socket* self, char* dst, size_t dstsz);

/**
 * Wrapper of send(2)
 * Send string to socket
 *
 * @param[in] self 
 * @param[in] str  send C string
 *
 * @return success to number of send size
 * @return failed to number of under of zero
 */
int 
socket_send_string(Socket* self, char const* str);

/**
 * Wrapper of send(2)
 * Send bytes to socket
 * 
 * @param[in] self  
 * @param[in] bytes pointer to memory of bytes
 * @param[in] size  number of size of bytes
 *
 * @return success to number of send size
 * @return failed to number of under of zero
 */
int 
socket_send_bytes(Socket* self, unsigned char const* bytes, size_t size);

#endif
