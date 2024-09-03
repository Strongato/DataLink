#ifndef SRV_USAGE_H
#define SRV_USAGE_H

void print_usage(char* argv[]);
int validate_args_and_print_usage(char* argv[], bool help, const char* const filepath, const unsigned short port);

#endif