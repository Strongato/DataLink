#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"

int send_hello(int sfd)
{
    char buff[4096] = {};

    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)buff;
    hdr->type = htonl(MSG_HELLO_REQ);
    hdr->len = htons(1); // number of aditional elements, in our case hello->proto

    dbproto_hello_req* const hello = (dbproto_hello_req* const)&hdr[1];
    hello->proto = htons(PROTO_VER);

    size_t write_size = sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_req);
    ssize_t bytes_written = write(sfd, buff, write_size);
    if (bytes_written == -1) { perror("Unable to write from buffer to sfd"); return STATUS_ERROR; }
    if (bytes_written != write_size) { fprintf(stderr, "Partial write: Expected %zu bytes from buff, but wrote %zd bytes into sfd\n", write_size, bytes_written); return STATUS_ERROR; }

    read(sfd, buff, sizeof(buff));

    hdr->type = ntohl(hdr->type);
    hdr->len = ntohs(hdr->len);

    if (hdr->type == MSG_ERROR) { printf("Protocol mismatch\n"); return STATUS_ERROR; }

    printf("Server connected, protocol v%d.\n", PROTO_VER);
    return STATUS_SUCCESS;
}

int send_employee(int sfd, const char* const add_string)
{
    char buff[4096] = {};

    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)buff;
    hdr->type = htonl(MSG_EMPLOYEE_ADD_REQ);
    hdr->len = htons(1);

    dbproto_employee_add_req* const employee = (dbproto_employee_add_req* const)&hdr[1];
    strncpy((char*)employee->data, add_string, sizeof(employee->data));

    size_t write_size = sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_add_req);
    ssize_t bytes_written = write(sfd, buff, write_size);
    if (bytes_written == -1) { perror("Unable to write from buffer to sfd"); return STATUS_ERROR; }
    if (bytes_written != write_size) { fprintf(stderr, "Partial write: Expected %zu bytes from buff, but wrote %zd bytes into sfd\n", write_size, bytes_written); return STATUS_ERROR; }

    read(sfd, buff, sizeof(buff));

    hdr->type = ntohl(hdr->type);
    hdr->len = ntohs(hdr->len);

    if (hdr->type == MSG_ERROR) { printf("Improper format for add employee string <name,address,hours> or something failed on server side\n"); return STATUS_ERROR; }

    if (hdr->type == MSG_EMPLOYEE_ADD_RESP) printf("Employee succesfully added\n");

    return STATUS_SUCCESS;
}

int remove_employees(int sfd, const char* const employee_string)
{
    char buff[4096] = {};

    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)buff;
    hdr->type = htonl(MSG_EMPLOYEE_DEL_REQ);
    hdr->len = htons(1);

    dbproto_employee_del_req* const employee = (dbproto_employee_del_req* const)&hdr[1];
    strncpy((char*)employee->data, employee_string, sizeof(employee->data));

    size_t write_size = sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_del_req);
    ssize_t bytes_written = write(sfd, buff, write_size);
    if (bytes_written == -1) { perror("Unable to write from buffer to sfd"); return STATUS_ERROR; }
    if (bytes_written != write_size) { fprintf(stderr, "Partial write: Expected %zu bytes from buff, but wrote %zd bytes into sfd\n", write_size, bytes_written); return STATUS_ERROR; }

    read(sfd, buff, sizeof(buff));

    hdr->type = ntohl(hdr->type);
    hdr->len = ntohs(hdr->len);

    if (hdr->type == MSG_ERROR) { printf("Improper format for removing employee string <name,address,hours> or something failed on server side\n"); return STATUS_ERROR; }

    if (hdr->type == MSG_EMPLOYEE_MISSING_RESP) { printf("Database is empty or the employee does not exist in the database\n"); return STATUS_ERROR; }

    if (hdr->type == MSG_EMPLOYEE_DEL_RESP) printf("Employee succesfully removed\n");

    return STATUS_SUCCESS;
}

int update_employees(int sfd, const char* const employee_string, const char* const update_string)
{
    char buff[4096] = {};

    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)buff;
    hdr->type = htonl(MSG_EMPLOYEE_UPDATE_REQ);
    hdr->len = htons(2);

    dbproto_employee_update_req* const employee = (dbproto_employee_update_req* const)&hdr[1];
    strncpy((char*)employee->data, employee_string, sizeof(employee->data));
    strncpy((char*)employee->update_hours_str, update_string, sizeof(employee->update_hours_str));

    size_t write_size = sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_update_req);
    ssize_t bytes_written = write(sfd, buff, write_size);
    if (bytes_written == -1) { perror("Unable to write from buffer to sfd"); return STATUS_ERROR; }
    if (bytes_written != write_size) { fprintf(stderr, "Partial write: Expected %zu bytes from buff, but wrote %zd bytes into sfd\n", write_size, bytes_written); return STATUS_ERROR; }

    read(sfd, buff, sizeof(buff));

    hdr->type = ntohl(hdr->type);
    hdr->len = ntohl(hdr->len);

    if (hdr->type == MSG_ERROR) { printf("Improper format for updating employee -u <hours> or something failed on server side\n"); return STATUS_ERROR; }

    if (hdr->type == MSG_EMPLOYEE_MISSING_RESP) { printf("Database is empty or the employee does not exist in the database\n"); return STATUS_ERROR; }

    if (hdr->type == MSG_EMPLOYEE_UPDATE_RESP) printf("Employee succesfully updated\n");

    return STATUS_SUCCESS;
}

void list_employee(const dbproto_employee_list_resp* const employee, const int i)
{
    printf("Employee %d\n", i);
    printf("\tName: %s\n", employee->name);
    printf("\tAddress: %s\n", employee->address);
    printf("\tHours: %u\n", employee->hours);
}

int list_employees(int sfd)
{
    char buff[4096] = {};

    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)buff;
    hdr->type = htonl(MSG_EMPLOYEE_LIST_REQ);
    hdr->len = htons(0);

    size_t write_size = sizeof(dbproto_hdr_t);
    ssize_t bytes_written = write(sfd, buff, write_size);
    if (bytes_written == -1) { perror("Unable to write from buffer to sfd"); return STATUS_ERROR; }
    if (bytes_written != sizeof(dbproto_hdr_t)) { fprintf(stderr, "Partial write: Expected %zu bytes from buff, but wrote %zd bytes into sfd\n", write_size, bytes_written); return STATUS_ERROR; }

    read(sfd, buff, sizeof(buff));

    hdr->type = ntohl(hdr->type);
    hdr->len = ntohs(hdr->len);

    if (hdr->type == MSG_EMPLOYEE_LIST_RESP)
    {
        printf("Listing employees\n");
        dbproto_employee_list_resp* const employee = (dbproto_employee_list_resp* const)&hdr[1];

        for (int i = 0; i < hdr->len; i++)
        {
            read(sfd, employee, sizeof(dbproto_employee_list_resp));
            employee->hours = ntohl(employee->hours); // Only hours needs to be unpacked, because the rest are chars

            list_employee(employee, i);
        }
    }
    else if (hdr->type == MSG_ERROR) { printf("MSG_ERROR received\n"); return STATUS_ERROR; }
    else { printf("Something on server side failed\n"); return STATUS_ERROR; }

    return STATUS_SUCCESS;
}