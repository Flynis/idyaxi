#ifndef _SOCKET_H_INCLUDED_
#define _SOCKET_H_INCLUDED_


#include <sys/socket.h>


/**
 * Opens socket for listening incoming connections.
 * @returns socket on success, X_FAILED otherwise.
*/
int socket_open_listening(struct sockaddr *addr, socklen_t addrlen);


/**
 * Accepts a socket from pending connections for the listening socket.
 * @returns socket on success, X_FAILED otherwise.
*/
int socket_accept(int listen_sock);


/**
 * Opens and connects UDP socket to local dns resolver.
 * @returns socket on success, X_FAILED otherwise.
*/
int socket_open_dns();


/**
 * Closes socket
*/
void socket_close(int sock);


#endif // _SOCKET_H_INCLUDED_
