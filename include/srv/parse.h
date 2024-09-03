#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144
/*
0x4c = 'L'
0x4c = 'L'
0x41 = 'A'
0x44 = 'D'
*/

struct dbheader_t
{
	unsigned int magic;
	unsigned short version;
	unsigned short count;
	unsigned int filesize;
};

struct employee_t
{
	char name[256];
	char address[256];
	unsigned int hours;
};

int create_db_header(struct dbheader_t** headerOut);
int validate_db_header(const int fd, struct dbheader_t** headerOut);

int parse_employee_string(char** name, char** addr, int* const int_hours, const char* const add_string);
void free_parse_employee_string(char** name, char** addr);

int read_employees(const int fd, const struct dbheader_t* const dbhdr, struct employee_t** employeesOut);
int add_employee(struct dbheader_t* const dbhdr, struct employee_t** employees, const char* const add_string);
int remove_employees(struct dbheader_t* const dbhdr, struct employee_t** employees, const char* const employee_string);
int update_employees(struct dbheader_t* const dbhdr, struct employee_t* employees, const char* const employee_string, const char* const update_hours_str);

int compare_employees(const void* a, const void* b);
int output_file(int* fd, struct dbheader_t* const dbhdr, struct employee_t* const employees, const char* const filepath);

#endif