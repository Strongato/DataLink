#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "comms.h"
#include "cli_usage.h"

int main(int argc, char* argv[])
{ 
    int ch = 0;
    bool list = false;
    bool remove = false;
    char* add_string = NULL;
    char* portarg = NULL;
    char* hostarg = NULL;
    char* employee_string = NULL;
    char* update_string = NULL;
    unsigned short port = 0;

    while ((ch = getopt(argc, argv, "a:p:h:le:u:r")) != -1)
    {
        switch (ch)
        {
            case 'a':
                add_string = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                break;
            case 'h':
                hostarg = optarg;
                break;
            case 'l':
                list = true;
                break;
            case 'e':
                employee_string = optarg;
                break;
            case 'u':
                update_string = optarg;
                break;
            case 'r':
                remove = true;
                break;
            case '?': // if none of the arguments are found '?' is returned by getopt
                printf("Unknown option -%c\n", ch);
                break;
        }
    }

    if (validate_args_and_print_usage(argv, port, hostarg, employee_string, update_string, remove) == STATUS_ERROR) return -1;

    const struct sockaddr_in server_info = { .sin_family = AF_INET, .sin_addr.s_addr = inet_addr(hostarg), .sin_port = htons(port) };

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) { perror("Unable to create socket fd"); return -1; }

    if (connect(sfd, (struct sockaddr*)&server_info, sizeof(server_info)) == -1) { fprintf(stderr, "Unable to connect to %s\n", hostarg); close(sfd); return -1; }

    if (send_hello(sfd) == STATUS_ERROR) { printf("Unable to send hello between client and host\n"); close(sfd); return -1; }

    if (add_string && send_employee(sfd, add_string) == STATUS_ERROR) { printf("Unable to add employee\n"); close(sfd); return -1; }

    if (employee_string)
    {
        if (remove && remove_employees(sfd, employee_string) == STATUS_ERROR) { printf("Unable to remove employee from database file\n"); close(sfd); return -1; }
        else if (update_string && update_employees(sfd, employee_string, update_string) == STATUS_ERROR) { printf("Unable to update database file\n"); close(sfd); return -1;}
    }

    if (list && list_employees(sfd) == STATUS_ERROR) { printf("Unable to list employees\n"); close(sfd); return -1; }

    close(sfd);
}