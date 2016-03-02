#ifndef SOCKET_H
#define SOCKET_H

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

#if defined(_WIN32) || defined(_WIN64)
# include "windows.h"
#else
# undef _BSD_SOURCE
# define _BSD_SOURCE 1 /* For netdb.h in cap/socket.h */
# undef __USE_POSIX
# define __USE_POSIX 1 /* For netdb.h in cap/socket.h */
# include <netdb.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#endif

typedef struct Socket Socket;

/**
 * @brief 
 *
 * @param self 
*/
void 
socket_display(Socket const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return 
*/
int 
socket_close(Socket* self);

/**
 * @brief 
 *
 * @param src  
 * @param mode 
 *
 * @return 
*/
Socket* 
socket_open(char const* src, char const* mode);

/**
 * @brief      
 *
 * @param      self
 *
 * @return     
 */
char const*
socket_host(Socket const* self);

/**
 * @brief      
 *
 * @param      self
 *
 * @return     
 */
char const*
socket_port(Socket const* self);

/**
 * @brief 
 *
 * @param self 
 *
 * @return pointer to Socket of client
*/
Socket* 
socket_accept(Socket const* self);

/**
 * @brief      
 *
 * @param      self
 * @param      dst 
 * @param[in]  dstsz
 *
 * @return     
 */
int
socket_recv_string(Socket* self, char* dst, size_t dstsz);

/**
 * @brief 
 *
 * @param self 
 * @param str  
 *
 * @return 
*/
int 
socket_send_string(Socket* self, char const* str);

int 
socket_send_bytes(Socket* self, unsigned char const* bytes, size_t size);

#endif
