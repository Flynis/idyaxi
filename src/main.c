#include <stdio.h>
#include <stdlib.h>


#include "cli.h"
#include "exceptions.h"
#include "logger.h"
#include "proxy.h"


int main(int argc, char const **argv) {
    int ret;
    ProxyOptions options;
    ret = proxy_options_from_cmd_line(argc, argv, &options);
    if(ret == X_FAILED) {
        puts("Failed to proccess cmd line args");
        return EXIT_FAILURE;
    }

    logger_init_consolelog(stdout);
    ret = logger_init_filelog("logs/log", 10 * 1024 * 1024, 255);
    if(ret == X_FAILED) {
        puts("Failed to init file logger");
        return EXIT_FAILURE;
    }

    Proxy p;
    ret = proxy_init(&p, &options);
    if(ret == X_FAILED) {
		puts("Failed to initialize proxy server");
        logger_destroy_filelog();
        return EXIT_FAILURE;
    }
    
    int status = EXIT_SUCCESS;
    int ret = proxy_listen(&p);
    if(ret == X_FAILED) {
		puts("Fatal proxy server error");
        status = EXIT_FAILURE;
    }

    proxy_destroy(&p);
    logger_destroy_filelog();
    return status;
}
