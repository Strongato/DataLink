#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define STATUS_SUCCESS 0
#define STATUS_ERROR  -1
#define STATUS_ERROR2 -2

#define PROTO_VER 2

typedef enum
{
    MSG_HELLO_REQ,
    MSG_HELLO_RESP,
    MSG_EMPLOYEE_LIST_REQ,
    MSG_EMPLOYEE_LIST_RESP,
    MSG_EMPLOYEE_ADD_REQ,
    MSG_EMPLOYEE_ADD_RESP,
    MSG_EMPLOYEE_DEL_REQ,
    MSG_EMPLOYEE_DEL_RESP,
    MSG_EMPLOYEE_UPDATE_REQ,
    MSG_EMPLOYEE_UPDATE_RESP,
    MSG_EMPLOYEE_MISSING_RESP,
    MSG_ERROR
} dbproto_type_e;

typedef struct
{
    dbproto_type_e type;
    uint16_t len; // number of additional elements, not bytes
} dbproto_hdr_t;

typedef struct
{
    uint16_t proto;
} dbproto_hello_req;

typedef struct
{
    uint16_t proto;
} dbproto_hello_resp;

typedef struct
{
    uint8_t data[1024];
} dbproto_employee_add_req;

typedef struct
{
    uint8_t data[1024];
} dbproto_employee_del_req;

typedef struct
{
    uint8_t proto;
} dbproto_employee_list_req;

typedef struct
{
	char name[256];
	char address[256];
	unsigned int hours;
} dbproto_employee_list_resp;

typedef struct
{
    uint8_t data[1024];
    uint8_t update_hours_str[16];
} dbproto_employee_update_req;

#endif