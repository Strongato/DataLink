#ifndef COMMS_H
#define COMMS_H

#include "common.h"

int send_hello(int sfd);
int send_employee(int sfd, const char* const add_string);
int remove_employees(int sfd, const char* const employee_string);
int update_employees(int sfd, const char* const employee_string, const char* const update_string);
void list_employee(const dbproto_employee_list_resp* const employee, const int i);
int list_employees(int sfd);

#endif