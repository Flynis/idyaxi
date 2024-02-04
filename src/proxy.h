#ifndef _PROXY_H_INCLUDED_
#define _PROXY_H_INCLUDED_


#include "proxy_options.h"


/**
 * Socks5 proxy.
*/
typedef struct Proxy {
    int listen_sock;
} Proxy;


/**
 * Starts to listen to incoming connections and serve its.
 * @returns 0 on success, X_FAILED otherwise.
*/
int proxy_listen(Proxy *p);


/**
 * Initializes a proxy p. Only use this function on an uninitialized proxy. 
 * Proxy that is initialized with this function must be destroyed 
 * with a call to proxy_destroy.
 * @param options the proxy options.
 * @see proxy_destroy()
 * @returns 0 on success, X_FAILED otherwise.
*/
int proxy_init(Proxy *p, const ProxyOptions *options);


/**
 * Destroys proxy p.
 * p must have been initialized with a call to proxy_init.
*/
void proxy_destroy(Proxy *p);


/**
 * Allocates and initializes a proxy. Proxy that is allocated 
 * with this function must be deleted with a call to proxy_delete.
 * @param options the proxy options.
 * @see proxy_delete()
 * @returns new proxy, or NULL on error.
*/
Proxy* proxy_new(const ProxyOptions *options);


/**
 * Deletes proxy p.
 * p must have been allocated with a call to proxy_new.
*/
void proxy_delete(Proxy *p);


#endif // _PROXY_H_INCLUDED_
