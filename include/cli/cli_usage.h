#ifndef CLI_USAGE_H
#define CLI_USAGE_H

void print_usage(char* argv[]);
int validate_args_and_print_usage(char* argv[], const unsigned short port, const char* const hostarg, const char* const employee_string, const char* const update_string, const bool remove, const bool add);

#endif
