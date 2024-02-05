#include "cli.h"


#include <arpa/inet.h>
#include <assert.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "exceptions.h"
#include "inet_limits.h"


#define DEFAULT_PORT 1080


typedef struct CliOptions {
    sa_family_t family;
    struct in_addr ipv4;
    struct in6_addr ipv6;
    uint16_t port;
} CliOptions;


static int parse_port(const char *arg, CliOptions *o) {
    int port = atoi(arg);
    if(port < 0 || port > PORT_MAX) {
        return X_FAILED;
    }
    o->port = port;
    return 0;
}


static int parse_ipv4(const char *arg, CliOptions *o) {
    o->family = AF_INET;
    int ret = inet_pton(AF_INET, arg, &o->ipv4);
    return (ret == 1) ? 0 : X_FAILED;
}


static int parse_ipv6(const char *arg, CliOptions *o) {
    o->family = AF_INET6;
    int ret = inet_pton(AF_INET6, arg, &o->ipv6);
    return (ret == 1) ? 0 : X_FAILED;
}


static size_t get_option_length(const char *arg) {
    if(arg[0] != '-') {
        return 0;
    }
    size_t len = 0;
    for(size_t i = 1; i < strlen(arg); i += 1) {
        if(arg[i] == '=') {
            break;
        }
        len += 1;
    }
    return len;
}


static int process_arg(const char *arg, size_t len, CliOptions *o) {
    switch(len)
    {
    case 0: return parse_port(arg, &o);

    case 1:
        switch(arg[1])
        {
        case '4': return parse_ipv4(arg, &o);
        case '6': return parse_ipv4(arg, &o);
        default: return X_FAILED;
        }
    
    default: return X_FAILED;
    }
}


static int fill_proxy_options(CliOptions *o, ProxyOptions *options) {
    struct sockaddr_in ipv4;
    struct sockaddr_in6 ipv6;
    struct sockaddr *addr;
    socklen_t addrlen;

    if(o->port == 0) {
        o->port = DEFAULT_PORT;
    }

    switch(o->family)
    {
    case 0:
        ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
        // fall through

    case AF_INET:
        if(o->family == AF_INET) {
            ipv4.sin_addr = o->ipv4;
        }
        ipv4.sin_family = AF_INET;
        ipv4.sin_port = htons(o->port);
        addr = &ipv4;
        addrlen = sizeof(struct sockaddr_in);
        break;

    case AF_INET6:
        ipv6.sin6_family = AF_INET6;
        ipv6.sin6_addr = o->ipv6;
        ipv6.sin6_port = htons(o->port);
        addr = &ipv6;
        addrlen = sizeof(struct sockaddr_in6);
        break;

    default: return X_FAILED;
    }

    return proxy_options_init(options, addr, addrlen);
}


int proxy_options_from_cmd_line(int argc, char **argv, ProxyOptions *options) {
    assert(argc > 0);
    assert(argv != NULL);
    assert(options != NULL);

    CliOptions o = {
        .family = 0,
        .port = 0
    };

    for(int i = 1; i < argc; i += 1) {
        char *arg = argv[i];
        size_t len = get_option_length(arg);
        int ret = process_arg(arg, len, &o);
        if(ret == X_FAILED) {
            puts("options: [-4=<ipv4_addr>] [-6=<ipv6_addr>] [port]");
            return 0;
        }
    }
    return fill_proxy_options(&o, options);
}
