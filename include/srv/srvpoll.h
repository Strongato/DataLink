#ifndef SRVPOLL_H
#define SRVPOLL_H

#include "parse.h"
#include "common.h"

#define MAX_CLIENTS 256
#define PORT 8080
#define BUFF_SIZE 4096

typedef enum
{
    STATE_NEW,
    STATE_HELLO,
    STATE_MSG,
    STATE_DISCONNECTED
} state_e;

typedef struct 
{
    int fd;
    state_e state;
    char buffer[BUFF_SIZE];
} clientstate_t;

int poll_loop(const unsigned short port, struct dbheader_t* const dbhdr, struct employee_t* employees, int dbfd, const char* const filepath);

void init_clients(clientstate_t* const states);
int find_free_slot(const clientstate_t* const states);
int find_slot_by_fd(const clientstate_t* const states, const int fd);

void fsm_general_reply(clientstate_t* const client, dbproto_hdr_t* const hdr, const dbproto_type_e type);

void fsm_reply_hello(clientstate_t* const client, dbproto_hdr_t* const hdr);
void fsm_reply_hello_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

void fsm_reply_add(clientstate_t* const client, dbproto_hdr_t* const hdr);
void fsm_reply_add_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

void fsm_reply_update(clientstate_t* const client, dbproto_hdr_t* const hdr);
void fsm_reply_update_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

void fsm_reply_missing_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

void send_employees(const struct dbheader_t* const dbhdr, struct employee_t* const employees, clientstate_t* const client);
void handle_client_fsm(struct dbheader_t* const dbhdr, struct employee_t** employees, clientstate_t* client, int dbfd, const char* const filepath);

#endif
