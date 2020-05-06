/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_WIN64)
# define _SOCKET_WINDOWS 1
#endif

#if defined(_SOCKET_WINDOWS)
# include <windows.h>
# define _WIN32_WINNT 0x501
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# undef _BSD_SOURCE
# define _BSD_SOURCE 1 /* cap: socket.h: netdb.h */
# undef __USE_POSIX
# define __USE_POSIX 1 /* cap: socket.h: netdb.h */
# undef __USE_XOPEN2K
# define __USE_XOPEN2K 1 /* cap: socket.h: netdb.h */
# include <netdb.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#endif

struct socket;
typedef struct socket socket_t;

/**
 * show parameters of socket to stderr
 *
 * @param[in] self
 */
void
sock_dump(const socket_t *self, FILE *fout);

/**
 * close socket
 * if self is NULL then don't anything
 *
 * @param[in] self
 *
 * @return success to number of zero
 * @return failed to under number of zero
 */
int32_t
sock_close(socket_t *self);

/**
 * open socket by source and mode like a fopen(3)
 * the open modes are:
 *
 * 		tcp-client
 * 		tcp-server
 *
 * @param[in] src  format of "host:port" of C string
 * @param[in] mode open mode
 *
 * @return pointer to dynamic allocate memory of struct cap_socket
 */
socket_t *
sock_open(const uint8_t *src, const uint8_t *mode);

/**
 * get host of C string by socket
 *
 * @param[in] self
 *
 * @return pointer to memory of C string
 */
const uint8_t *
sock_getc_host(const socket_t *self);

/**
 * get port of C string by socket
 *
 * @param[in] self
 *
 * @return pointer to memory of C string
 */
const uint8_t *
sock_getc_port(const socket_t *self);

/**
 * wrapper of accept(2)
 * get new socket by self socket
 *
 * @param[in] self
 *
 * @return pointer to dynamic allocate memory of struct cap_socket of client
 */
socket_t *
sock_accept(socket_t *self);

/**
 * wrapper of recv(2)
 * recv string from socket
 *
 * @param[in] self
 * @param[in] dst pointer to memory of destination buffer
 * @param[in] dstsz number of size of destination buffer
 *
 * @return success to number of recv size
 * @return failed to number of under of zero
 */
int32_t
sock_recv_str(socket_t *self, uint8_t *dst, int32_t dstsz);

/**
 * wrapper of send(2)
 * send string to socket
 *
 * @param[in] self
 * @param[in] str send C string
 *
 * @return success to number of send size
 * @return failed to number of under of zero
 */
int32_t
sock_send_str(socket_t *self, const uint8_t *str);

/**
 * wrapper of send(2)
 * send bytes to socket
 *
 * @param[in] self
 * @param[in] bytes pointer to memory of bytes
 * @param[in] size number of size of bytes
 *
 * @return success to number of send size
 * @return failed to number of under of zero
 */
int32_t
sock_send(socket_t *self, const uint8_t *bytes, int32_t size);

/**
 * set error
 *
 * @param[in] *self
 * @param[in] *fmt
 */
void
sock_set_error(socket_t *self, const char *fmt, ...);

/**
 * get error message
 *
 * @param[in] *self
 *
 * @return poitner to C strings
 */
const char *
sock_getc_error(const socket_t *self);