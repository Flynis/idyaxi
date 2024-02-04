#include "cli.h"


#include <arpa/inet.h>
#include <assert.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "exceptions.h"
#include "inet_limits.h"


#define DEFAULT_PORT 1080


typedef enum ArgType {
    CLI_IPV4 = 1,
    CLI_IPV6,
    CLI_PORT
} ArgType;


static void print_help(void) {
    puts("options: [ipv4/ipv6] [port]");
}


static void print_failed(void) {
    puts("Process cmd line args failed");
}


static int set_port(int port, ProxyOptions *options) {
    if(port < 0 || port > PORT_MAX) {
        print_help();
        return X_FAILED;
    }
    if(options->addr == NULL) {
        struct sockaddr_in ipv4;
        ipv4.sin_family = AF_INET;
        ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
        ipv4.sin_port = htons(port);
        int ret = proxy_options_init(options, &ipv4, sizeof(ipv4));
        if(ret == X_FAILED) {
            print_failed();
            return X_FAILED;
        }
        return 0;
    } else {
        if(options->addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = options->addr;
            addr->sin_port = htons(port);
            return 0;
        } else if(options->addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *addr = options->addr;
            addr->sin6_port = htons(port);
            return 0;
        }
    }
    return X_FAILED;
}


static int process_arg(char const *arg, ArgType arg_type, ProxyOptions *options) {
    int ret;
    switch (arg_type)
    {
    case CLI_IPV4:
        struct sockaddr_in ipv4;
        ipv4.sin_family = AF_INET;
        ret = inet_pton(AF_INET, arg, &ipv4.sin_addr);
        if(ret == 0) {
            print_help();
            return X_FAILED;
        }
        ret = proxy_options_init(options, &ipv4, sizeof(ipv4));
        if(ret == X_FAILED) {
            print_failed();
            return X_FAILED;
        }
        return 0;

    case CLI_IPV6:
        struct sockaddr_in6 ipv6;
        ipv6.sin6_family = AF_INET6;
        ret = inet_pton(AF_INET6, arg, &ipv6.sin6_addr);
        if(ret == 0) {
            print_help();
            return X_FAILED;
        }
        ret = proxy_options_init(options, &ipv6, sizeof(ipv6));
        if(ret == X_FAILED) {
            print_failed();
            return X_FAILED;
        }
        return 0;

    case CLI_PORT:
        int port = atoi(arg);
        return set_port(port, options);

    default: return X_FAILED;
    }
}


int proxy_options_from_cmd_line(int argc, char **argv, ProxyOptions *options) {
    assert(argc > 0);
    assert(argv != NULL);
    assert(options != NULL);

    options->addr = NULL;
    options->addrlen = 0;
    int ret = 0;

    if(argc > 1) {
        ArgType type = CLI_PORT;
        for(int i = 0; i < strlen(argv[1]); i++) {
            if(argv[1] == '.') {
                type = CLI_IPV4;
                break;
            }
            if(argv[1] == ':') {
                type = CLI_IPV6;
                break;
            }
        }
        ret = process_arg(argv[1], type, options);
        if(ret == X_FAILED) {
            return X_FAILED;
        }
        if(type == CLI_PORT) {
            return 0;
        }
        if(argc > 2) {
            return process_arg(argv[2], CLI_PORT, options);
        }
    }

    return set_port(DEFAULT_PORT, options);
}
