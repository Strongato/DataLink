#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "common.h"
#include "file.h"
#include "parse.h"
#include "srvpoll.h"
#include "srv_usage.h"

int main(int argc, char* argv[])
{ 
    int ch = 0;
    bool newfile = false;
    bool help = false;
	char* filepath = NULL;
    char* portarg = NULL;
    unsigned short port = 0;
    
    while ((ch = getopt(argc, argv, "nhf:p:")) != -1) // argument followed by : means the option has a required argument
    {
        switch(ch)
        {
            case 'n':
                newfile = true;
                break;
            case 'h':
                help = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                break;
            case '?': // if none of the arguments are found '?' is returned by getopt
                printf("Unknown option -%c\n", ch);
                break;
        }
    }
    if (validate_args_and_print_usage(argv, help, filepath, port) == STATUS_ERROR) return -1;

    int dbfd = -1;
    struct dbheader_t* dbhdr = NULL;
    if (newfile)
    {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) { printf("Unable to create database file\n"); return -1; }

        if (create_db_header(&dbhdr) == STATUS_ERROR) { printf("Unable to create database header\n"); close(dbfd); return -1; }
        if (output_file(&dbfd, dbhdr, NULL, filepath) != STATUS_SUCCESS) { printf("Unable to output database header to database file\n"); close(dbfd); free(dbhdr); return -1; }
    }
    else
    {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) { printf("Unable to open database file\n"); return -1; }

        if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) { printf("Unable to validate database header\n"); close(dbfd); return -1; }
    }

    struct employee_t* employees = NULL;
    if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) { printf("Unable to read employees\n"); close(dbfd); free(dbhdr); return -1; }

    poll_loop(port, dbhdr, employees, dbfd, filepath); // infinite loop

    close(dbfd);
    free(dbhdr);
    free(employees);
    return 0;
}