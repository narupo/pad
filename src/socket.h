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
# include "windows.h"
#else
# define _BSD_SOURCE
# include <netdb.h>
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

#endif
