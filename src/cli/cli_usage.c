#include <stdio.h>
#include <stdbool.h>

#include "common.h"

void print_usage(char* argv[])
{
    printf("\n-h  - (required) Host IP address\n");
    printf("    Usage: %s -h <ip>\n", argv[0]);
    printf("-p  - (required) Set the port number\n");
    printf("    Usage: %s -p <port>\n", argv[0]);
    printf("-l  - List employees from the database file\n");
    printf("    Usage: %s -l\n", argv[0]);
    printf("-a  - Add an employee to the database file\n");
    printf("    Usage: %s -a <name,address,hours>\n", argv[0]);
    printf("-e  - Specify employee (required for -r and -u options)\n");
    printf("    Usage: %s -e <name,address,hours>\n", argv[0]);
    printf("-r  - Remove an employee from the database file (-e option required)\n");
    printf("    Usage: %s -r\n", argv[0]);
    printf("-u  - Update an employee's hours worked (-e option required)\n");
    printf("    Usage: %s -u <hours>\n", argv[0]);
}

int validate_args_and_print_usage(char* argv[], const unsigned short port, const char* const hostarg, const char* const employee_string, const char* const update_string, const bool remove)
{
    bool help = false;
    if (port < 1024) { printf("Port number is a required argument and must be in range [1024,65535]\n"); help = true; }
    if (!hostarg) { printf("Host ip adress is a required argument\n"); help = true; }

    if (employee_string)
    {
        if (!remove && !update_string) { printf("Employee name -e is set but no additional parameters like -u or -r were provided\n"); help = true; }
    }
    else
    {
        if (remove) { printf("Employee name -e is a required argument when using -r\n"); help = true; }
        if (update_string) { printf("Employee name -e is a required argument when using -u\n"); help = true; }
    }

    if (help) { print_usage(argv); return STATUS_ERROR; }
    return STATUS_SUCCESS;
}