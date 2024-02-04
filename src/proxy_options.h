#ifndef _PROXY_OPTIONS_H_INCLUDED_
#define _PROXY_OPTIONS_H_INCLUDED_


#include <stdint.h>
#include <sys/socket.h>


/**
 * Struct of proxy input options.
*/
typedef struct ProxyOptions {
    const struct sockaddr *addr;
    socklen_t addrlen;
} ProxyOptions;


/**
 * Initializes proxy options o. 
 * Only use this function on uninitialized proxy options. 
 * Proxy options that is initialized with this function 
 * must be destroyed with a call to proxy_options_destroy.
 * @see proxy_options_destroy()
 * @returns 0 on success, X_FAILED otherwise.
*/
int proxy_options_init(ProxyOptions *o, const struct sockaddr *addr, socklen_t addrlen);


/**
 * Destroys proxy options o.
 * o must have been initialized with a call to proxy_options_init.
*/
void proxy_options_destroy(ProxyOptions *o);


#endif // _PROXY_OPTIONS_H_INCLUDED_
