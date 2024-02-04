#include <stdio.h>
#include <stdlib.h>


#include "cli.h"
#include "exceptions.h"
#include "proxy.h"


int main(int argc, char const **argv) {
    ProxyOptions options;
    int ret = proxy_options_from_cmd_line(argc, argv, &options);
    if(ret == X_FAILED) {
        return EXIT_FAILURE;
    }

    Proxy p;
    ret = proxy_init(&p, &options);
    if(ret == X_FAILED) {
		puts("Failed to initialize proxy server");
        return EXIT_FAILURE;
    }
    
    int status = EXIT_SUCCESS;
    int ret = proxy_listen(&p);
    if(ret == X_FAILED) {
		puts("Fatal proxy server error");
        status = EXIT_FAILURE;
    }

    proxy_destroy(&p);
    return status;
}
