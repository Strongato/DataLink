#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#include "common.h"
#include "parse.h"
#include "srvpoll.h"

int poll_loop(const unsigned short port, struct dbheader_t* const dbhdr, struct employee_t* employees, int dbfd, const char* const filepath)
{
    clientstate_t clientStates[MAX_CLIENTS] = {};
    init_clients(clientStates);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) { perror("Unable to create socket listed_fd"); return STATUS_ERROR; }
    
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) { perror("Unable to set socket options"); close(listen_fd); return STATUS_ERROR; }

    struct sockaddr_in server_addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(port) };

    struct sockaddr_in client_addr = {};
    socklen_t client_len = sizeof(client_addr);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) { perror("Unable to bind to server_addr"); close(listen_fd); return STATUS_ERROR; }

    if (listen(listen_fd, 10) == -1) { perror("Unable to listen on listen_fd"); close(listen_fd); return STATUS_ERROR; }

    printf("Server listening on port %d\n", port);

    struct pollfd fds[MAX_CLIENTS + 1] = {};
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;
    int nfds = 1; // number of fd's

    while (1)
    {
        for (int i = 0, j = 1; i < MAX_CLIENTS; i++) // j = 1 because we already have listen_fd in fds[0]
        {
            if (clientStates[i].fd != -1)
            {
                fds[j].fd = clientStates[i].fd;
                fds[j].events = POLLIN;
                j++;
            }
        }

        // Wait for an event on one of the sockets
        // poll returns the number of events waiting
        int n_events = poll(fds, nfds, -1); // -1 means no timeout
        if (n_events == -1) { perror("poll failed"); close(listen_fd); return STATUS_ERROR; }

        int conn_fd = -1;
        if (fds[0].revents & POLLIN)
        {
            if ((conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len)) == -1) { perror("Unable to accept client"); close(listen_fd); return STATUS_ERROR; }
            printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            int free_slot = find_free_slot(clientStates);
            if (free_slot != -1)
            {
                clientStates[free_slot].fd = conn_fd;
                clientStates[free_slot].state = STATE_HELLO;
                nfds++;
                printf("Slot %d has fd %d\n", free_slot, clientStates[free_slot].fd);
            }
            else
            {
                printf("Server full: closing new connection\n");
                close(conn_fd);
            }

            n_events--;
        }

        // check each client for read/write activity
        for (int i = 1; i <= nfds && n_events > 0; i++) // start from 1 to skip listen_fd
        {
            if (fds[i].revents & POLLIN) // if we received events
            {
                int slot = find_slot_by_fd(clientStates, fds[i].fd);

                ssize_t bytes_read = read(fds[i].fd, clientStates[slot].buffer, sizeof(clientStates[slot].buffer) - 1);
                if (bytes_read > 0)
                {
                    clientStates[slot].buffer[bytes_read] = '\0';
                    handle_client_fsm(dbhdr, &employees, &clientStates[slot], dbfd, filepath);
                }
                else
                {
                    close(fds[i].fd);
                    if (slot != -1)
                    {
                        clientStates[slot].fd = -1;
                        clientStates[slot].state = STATE_DISCONNECTED;
                        memset(clientStates[i].buffer, '\0', BUFF_SIZE);
                        // probably not needed because everything that is read gets null terminated, so there is no way to access the old data
                        printf("Client with fd %d disconnected or error\n", fds[i].fd);
                        nfds--;
                    }
                    else { printf("Tried to close fd that does not exist?\n"); }
                }

                n_events--;
            }
        }
    }
    close(listen_fd);
}

void init_clients(clientstate_t* const states)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        states[i].fd = -1; // -1 indicates a free slot
        states[i].state = STATE_NEW;
        memset(states[i].buffer, '\0', BUFF_SIZE);
    }
}

int find_free_slot(const clientstate_t* const states)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (states[i].fd == -1) return i;
    }
    return -1; // no free slot found
}

int find_slot_by_fd(const clientstate_t* const states, const int fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (states[i].fd == fd) return i;
    }
    return -1; // not found
}

void fsm_general_reply(clientstate_t* const client, dbproto_hdr_t* const hdr, const dbproto_type_e type)
{
    hdr->type = htonl(type);
    hdr->len = htons(0); // sending 0 aditional elements

    write(client->fd, hdr, sizeof(dbproto_hdr_t));
}

void fsm_reply_hello(clientstate_t* const client, dbproto_hdr_t* const hdr)
{
    hdr->type = htonl(MSG_HELLO_RESP);
    hdr->len = htons(1); // sending 1 aditional element

    dbproto_hello_resp* const hello = (dbproto_hello_resp* const)&hdr[1];
    hello->proto = htons(PROTO_VER);

    write(client->fd, hdr, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_resp));
}
void fsm_reply_hello_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

void fsm_reply_add(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_ADD_RESP); }
void fsm_reply_add_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

void fsm_reply_del(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_DEL_RESP); }
void fsm_reply_del_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

void fsm_reply_update(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_UPDATE_RESP); }
void fsm_reply_update_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

void fsm_reply_missing_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_MISSING_RESP); }

void send_employees(const struct dbheader_t* const dbhdr, struct employee_t* const employees, clientstate_t* const client)
{
    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)client->buffer;
    hdr->type = htonl(MSG_EMPLOYEE_LIST_RESP);
    hdr->len = htons(dbhdr->count);

    ssize_t bytes_written = write(client->fd, hdr, sizeof(dbproto_hdr_t));
    if (bytes_written == -1) { perror("Unable to write from header to client->fd"); return; }
    if (bytes_written != sizeof(dbproto_hdr_t)) { fprintf(stderr, "Partial write: Expected %zu bytes from buff, but wrote %zd bytes into sfd\n", sizeof(dbproto_hdr_t), bytes_written); return; }

    dbproto_employee_list_resp* const employee = (dbproto_employee_list_resp* const)&hdr[1];

    for (int i = 0; i < dbhdr->count; i++)
    {
        strncpy(employee->name, employees[i].name, sizeof(employee->name));
        strncpy(employee->address, employees[i].address, sizeof(employee->address));
        employee->hours = htonl(employees[i].hours);

        write(client->fd, employee, sizeof(dbproto_employee_list_resp));
        employee->hours = ntohl(employees[i].hours);
    }
}

void handle_client_fsm(struct dbheader_t* const dbhdr, struct employee_t** employees, clientstate_t* client, int dbfd, const char* const filepath)
{
    dbproto_hdr_t* const hdr = (dbproto_hdr_t* const)client->buffer;

    hdr->type = ntohl(hdr->type);
    hdr->len = ntohs(hdr->len);

    if (client->state == STATE_HELLO)
    {
        if (hdr->type != MSG_HELLO_REQ || hdr->len != 1) { printf("Didnt receive MSG_HELLO in HELLO state\n"); fsm_reply_hello_err(client, hdr); return; }

        dbproto_hello_req* const hello = (dbproto_hello_req* const)&hdr[1]; // checking the aditional elements sent
        hello->proto = ntohs(hello->proto); // only once in this case
        if (hello->proto != PROTO_VER) { printf("Protocol mismatch\n"); fsm_reply_hello_err(client, hdr); return; }

        fsm_reply_hello(client, hdr);
        client->state = STATE_MSG;
        printf("Client upgraded to STATE_MSG\n");
    }

    if (client->state == STATE_MSG)
    {
        if (hdr->type == MSG_EMPLOYEE_ADD_REQ)
        {
            dbproto_employee_add_req* const employee = (dbproto_employee_add_req* const)&hdr[1];

            if (add_employee(dbhdr, employees, employee->data) == STATUS_SUCCESS)
            {
                int ret = output_file(&dbfd, dbhdr, *employees, filepath);
                if (ret != STATUS_SUCCESS) { printf("Unable to write to database file\nUnable to add employee\n"); fsm_reply_add_err(client, hdr); return; }
                
                printf("Added employee: %s\n", employee->data);
                fsm_reply_add(client, hdr);
            }
            else
            {
                printf("Unable to add employee: %s\n", employee->data);
                fsm_reply_add_err(client, hdr);
                return;
            }
        }

        if (hdr->type == MSG_EMPLOYEE_DEL_REQ)
        {
            dbproto_employee_del_req* const employee = (dbproto_employee_del_req* const)&hdr[1];

            int ret = remove_employees(dbhdr, employees, employee->data);
            if (ret == STATUS_SUCCESS)
            {
                int ret = output_file(&dbfd, dbhdr, *employees, filepath);
                if (ret != STATUS_SUCCESS) { printf("Unable to write to database file\nUnable to remove employee\n"); fsm_reply_add_err(client, hdr); return; }
                
                printf("Removed employee: %s\n", employee->data);
                fsm_reply_del(client, hdr);
            }
            else if (ret == STATUS_ERROR)
            {
                printf("Unable to remove employee: %s\n", employee->data);
                fsm_general_reply_missing_err(client, hdr);
                return;
            }
            else
            {
                printf("Unable to remove employee: %s\n", employee->data);
                fsm_reply_del_err(client, hdr);
                return;
            }
        }

        if (hdr->type == MSG_EMPLOYEE_UPDATE_REQ)
        {
            dbproto_employee_update_req* const employee = (dbproto_employee_update_req* const)&hdr[1];

            int ret = update_employees(dbhdr, *employees, employee->data, employee->update_hours_str);
            if (ret == STATUS_SUCCESS)
            {
                int ret = output_file(&dbfd, dbhdr, *employees, filepath);
                if (ret != STATUS_SUCCESS) { printf("Unable to write to database file\nUnable to remove employee\n"); fsm_reply_add_err(client, hdr); return; }

                printf("Updated employee: %s\n", employee->data);
                fsm_reply_update(client, hdr);
            }
            else if (ret == STATUS_ERROR)
            {
                printf("Unable to update employees: %s\n", employee->data);
                fsm_general_reply_missing_err(client, hdr);
                return;
            }
            else
            {
                printf("Unable to update employee: %s\n", employee->data);
                fsm_reply_update_err(client, hdr);
                return;
            }
        }

        if (hdr->type == MSG_EMPLOYEE_LIST_REQ)
        {
            printf("Listing employees\n");
            send_employees(dbhdr, *employees, client);
        }
    }
}
