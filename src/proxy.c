#include "proxy.h"


#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>


#include "exceptions.h"


static volatile bool running = false; // for signal handlers


static void proxy_stop() {
    running = false;
}


static void sig_handler(int signal) {
    proxy_stop();
}


static int set_signal_handlers() {

}


int proxy_listen(Proxy *p) {
    return 0;
}


int proxy_init(Proxy *p, const ProxyOptions *options) {
    return 0;
}


void proxy_destroy(Proxy *p) {

}


Proxy* proxy_new(const ProxyOptions *options) {
    assert(options != NULL);

    Proxy *p = malloc(sizeof(Proxy));
    if(p == NULL) {
        return NULL;
    }

    int ret = proxy_init(p, options);
    if(ret == X_FAILED) {
        free(p);
        return NULL;
    }
    return p;
}


void proxy_delete(Proxy *p) {
    proxy_destroy(p);
    free(p);
}
