#include <stdio.h>
#include <stdbool.h>

#include "common.h"

void print_usage(char* argv[])
{
    printf("\n-h  - List all possible options and their use cases\n");
    printf("    Usage: %s -h\n", argv[0]);
    printf("-f  - (required) Path to the database file\n");
    printf("    Usage: %s -f <database file>\n", argv[0]);
    printf("-p  - (required) Set the port number\n");
    printf("    Usage: %s -p <port> (port number must be in the range [1024, 65535])\n", argv[0]);
    printf("-n  - Create a new database file\n");
    printf("    Usage: %s -n\n", argv[0]);
}

int validate_args_and_print_usage(char* argv[], bool help, const char* const filepath, const unsigned short port)
{
    if (help == true) { print_usage(argv); return STATUS_ERROR; }
    if (filepath == NULL) { printf("Filepath is a required argument\n"); help = true; }
    if (port < 1024) { printf("Port number is a required argument and must be in range [1024,65535]\n"); help = true; }
    if (help == true) { print_usage(argv); return STATUS_ERROR; }
    return 0;
}