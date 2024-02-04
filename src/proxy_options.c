#include "proxy_options.h"


#include <assert.h>
#include <stdlib.h>
#include <string.h>


#include "exceptions.h"


int proxy_options_init(ProxyOptions *o, const struct sockaddr *addr, socklen_t addrlen) {
    assert(o != NULL);
    assert(addr != NULL);
    assert(addrlen > 0);
    
    o->addr = malloc(addrlen);
    if(o->addr == NULL) {
        return X_FAILED;
    }
    memcpy(o->addr, addr, addrlen);
    o->addrlen = addrlen;
    return 0;
}


void proxy_options_destroy(ProxyOptions *o) {
    free(o->addr);
}
