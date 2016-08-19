/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef SOCKET_H
#define SOCKET_H

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h> 
#include <ctype.h> 

#if defined(_WIN32) || defined(_WIN64)
#  define _CAP_WINDOWS 1
#endif

#if defined(_CAP_WINDOWS)
#  include <windows.h>
#else
#  undef _BSD_SOURCE
#  define _BSD_SOURCE 1 /* cap: socket.h: netdb.h */
#  undef __USE_POSIX
#  define __USE_POSIX 1 /* cap: socket.h: netdb.h */
#  undef __USE_XOPEN2K
#  define __USE_XOPEN2K 1 /* cap: socket.h: netdb.h */
#  include <netdb.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

struct cap_socket;

/**
 * Display parameters of socket
 *
 * @param[in] self
 */
void
cap_sockdisp(const struct cap_socket *self);

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
cap_sockclose(struct cap_socket *self);

/**
 * Open socket by source and mode like a fopen(3)
 * The open modes are:
 *
 * 		tcp-client
 * 		tcp-server
 *
 * @param[in] src  format of "host:port" of C string
 * @param[in] mode open mode
 *
 * @return pointer to dynamic allocate memory of struct cap_socket
 */
struct cap_socket *
cap_sockopen(const char *src, const char *mode);

/**
 * Get host of C string by socket
 *
 * @param[in] self
 *
 * @return pointer to memory of C string
 */
const char *
cap_sockhost(const struct cap_socket *self);

/**
 * Get port of C string by socket
 *
 * @param[in] self
 *
 * @return pointer to memory of C string
 */
const char *
cap_sockport(const struct cap_socket *self);

/**
 * Wrapper of accept(2)
 * Get new socket by self socket
 *
 * @param[in] self
 *
 * @return pointer to dynamic allocate memory of struct cap_socket of client
 */
struct cap_socket *
cap_sockaccept(const struct cap_socket *self);

/**
 * Wrapper of recv(2)
 * Recv string from socket
 *
 * @param[in] self
 * @param[in] dst pointer to memory of destination buffer
 * @param[in] dstsz number of size of destination buffer
 *
 * @return success to number of recv size
 * @return failed to number of under of zero
 */
int
cap_sockrecvstr(struct cap_socket *self, char *dst, size_t dstsz);

/**
 * Wrapper of send(2)
 * Send string to socket
 *
 * @param[in] self
 * @param[in] str send C string
 *
 * @return success to number of send size
 * @return failed to number of under of zero
 */
int
cap_socksendstr(struct cap_socket *self, const char *str);

/**
 * Wrapper of send(2)
 * Send bytes to socket
 *
 * @param[in] self
 * @param[in] bytes pointer to memory of bytes
 * @param[in] size number of size of bytes
 *
 * @return success to number of send size
 * @return failed to number of under of zero
 */
int
cap_socksend(struct cap_socket *self, const unsigned char *bytes, size_t size);

#endif

