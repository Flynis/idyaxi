#ifndef _CLI_H_INCLUDED_
#define _CLI_H_INCLUDED_


#include "proxy_options.h"

/**
 * Process cmd line args and returns proxy options.
 * @param[out] options a proxy options.
 * @param argc a cmd line args count.
 * @param argv cmd line args values.
 * @returns 0 on success, X_FAILED otherwise.
*/
int proxy_options_from_cmd_line(int argc, char **argv, ProxyOptions *options);


#endif // _CLI_H_INCLUDED_
