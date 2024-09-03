#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "common.h"
#include "parse.h"

int create_db_header(struct dbheader_t** headerOut)
{
	struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if (!header) { perror("Unable to allocate memory for variable header"); return STATUS_ERROR; }

    header->version = PROTO_VER;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}

int validate_db_header(const int fd, struct dbheader_t** headerOut) // does not return fd location to start of file
{
    if (fd < 0) { printf("Invalid file descriptor received by the user\n"); return STATUS_ERROR; }

    struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
    if (!header) { perror("Unable to allocate memory for variable header"); return STATUS_ERROR; }

    ssize_t bytes_read = read(fd, header, sizeof(struct dbheader_t));
    if (bytes_read == -1) { perror("Failed to read from fd into variable header"); free(header); EXIT_FAILURE; }
    if (bytes_read != sizeof(struct dbheader_t)) { fprintf(stderr, "Partial read: Expected %zu bytes, but read %zd bytes from fd into variable header\n", sizeof(struct dbheader_t), bytes_read); free(header); EXIT_FAILURE; }

    header->version = ntohs(header->version); // networks to header short (because type of variable version is unsigned short)
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic); // variable magic is of type unsigned int but can be used with ntohl because it expects a 32bit type (long used to be 32bit and thats where it got its name)
    header->filesize = ntohl(header->filesize);

    if (header->version != PROTO_VER) { fprintf(stderr, "Wrong header version, expected version %d, gotten %u\n", PROTO_VER, header->version); free(header); return STATUS_ERROR; }
    if (header->magic != HEADER_MAGIC) { fprintf(stderr, "Wrong header magic\n"); free(header); return STATUS_ERROR; }

    struct stat dbstat = {};
    if (fstat(fd, &dbstat) == -1) { perror("fstat failed"); free(header); return EXIT_FAILURE; }
    if (header->filesize != dbstat.st_size) { fprintf(stderr, "header->filesize does not match the actual filesize\n"); free(header); return STATUS_ERROR; }

    *headerOut = header;
    return STATUS_SUCCESS;
}

int read_employees(const int fd, const struct dbheader_t* const dbhdr, struct employee_t** employeesOut)
{
    if (fd < 0) { printf("Invalid file descriptor received by the user\n"); return STATUS_ERROR; }

    struct employee_t* employees = calloc(dbhdr->count, sizeof(struct employee_t));
    if (!employees) { perror("Unable to allocate memory for employees"); return STATUS_ERROR; }

    // since we already read the header with validate_db_header or created it with create_db_header we are now pointing to where employees will start
    ssize_t bytes_read = read(fd, employees, dbhdr->count * sizeof(struct employee_t));
    if (bytes_read == -1) { perror("Failed to read from fd into employees array"); free(employees); EXIT_FAILURE; }
    if (bytes_read != dbhdr->count * sizeof(struct employee_t)) { fprintf(stderr, "Partial read: Expected %zu bytes, but read %zd bytes into employees array\n", sizeof(struct employee_t), bytes_read); free(employees); EXIT_FAILURE; }

    // name and address from struct employee_t are arrays of chars which are 1 byte long, so they dont need to be converted
    for (int i = 0; i < dbhdr->count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}

int parse_employee_string(char** name, char** addr, int* const int_hours, const char* const add_string)
{
    // we need to copy add_string to tmp variable because strtok_r will modify the original string
    char str[strlen(add_string) + 1]; // we got variable add_string from getopt which returns a pointer to our command line argument which is null terminated
    strncpy(str, add_string, strlen(add_string) + 1); // we copied the whole string include the \0 (that + 1)
    
    char* saveptr = NULL;
    char* token = NULL;

    token = strtok_r(str, ",", &saveptr);
    if (!token) { printf("Invalid string employee format\n"); return STATUS_ERROR; }
    *name = strdup(token);
    if (!(*name)) { printf("Memory allocation failed for name token"); return STATUS_ERROR; }

    token = strtok_r(NULL, ",", &saveptr);
    if (!token) { printf("Invalid string employee format\n"); return STATUS_ERROR; }
    *addr = strdup(token);
    if (!(*addr)) { printf("Memory allocation failed for name token"); return STATUS_ERROR; }

    token = strtok_r(NULL, ",", &saveptr);
    if (!token) { printf("Invalid string employee format\n"); return STATUS_ERROR; }
    int hours = atoi(token);
    if (!hours && strcmp(token, "0")) { printf("ATOI failed\n"); return STATUS_ERROR; }
    *int_hours = hours;

    return STATUS_SUCCESS;
}

void free_parse_employee_string(char** name, char** addr)
{
    if (*name) { free(*name); *name = NULL; }

    if (*addr) { free(*addr); *addr = NULL; }
}

int add_employee(struct dbheader_t* const dbhdr, struct employee_t** employees, const char* const add_string)
{
    char* name = NULL;
    char* addr = NULL;
    int hours = 0;
    if (parse_employee_string(&name, &addr, &hours, add_string) == STATUS_ERROR) { free_parse_employee_string(&name, &addr); return STATUS_ERROR; }

    dbhdr->count++;
    struct employee_t* tmp = realloc(*employees, dbhdr->count * sizeof(struct employee_t));
    if (!tmp) { perror("Unable to reallocate memory for employees"); return STATUS_ERROR; }
    *employees = tmp;

    struct employee_t* const employees_ptr = *employees;
    strncpy(employees_ptr[dbhdr->count-1].name, name, sizeof(employees_ptr[dbhdr->count-1].name));
    strncpy(employees_ptr[dbhdr->count-1].address, addr, sizeof(employees_ptr[dbhdr->count-1].address));
    employees_ptr[dbhdr->count-1].hours = hours;

    free_parse_employee_string(&name, &addr);

    return STATUS_SUCCESS;
}

int remove_employees(struct dbheader_t* const dbhdr, struct employee_t** employees, const char* const employee_string)
{
    if (dbhdr->count == 0) { printf("Trying to remove an employee from an empty database\n"); return STATUS_ERROR; }
    
    char* name = NULL;
    char* addr = NULL;
    int hours = 0;
    if (parse_employee_string(&name, &addr, &hours, employee_string) == STATUS_ERROR) { free_parse_employee_string(&name, &addr); return STATUS_ERROR2; }

    struct employee_t* tmp_employees = calloc(dbhdr->count, sizeof(struct employee_t));
    if (!tmp_employees) { perror("Unable to allocate memory for tmp_employees"); free_parse_employee_string(&name, &addr); return STATUS_ERROR2; }

    struct employee_t* const employees_ptr = *employees;

    const int current_size = dbhdr->count;
    for (int i = 0, j = 0; i < current_size; i++)
    {
        if (!strcmp(name, employees_ptr[i].name) && !strcmp(addr, employees_ptr[i].address) && hours == employees_ptr[i].hours)
        {
            dbhdr->count--;
        }
        else
        {
            tmp_employees[j] = employees_ptr[i];
            j++;
        }
    }

    free(*employees);
    *employees = tmp_employees;
    free_parse_employee_string(&name, &addr);

    if (current_size == dbhdr->count) { printf("Employee does not exist in the database\n"); return STATUS_ERROR; }

    return STATUS_SUCCESS;
}

int update_employees(struct dbheader_t* const dbhdr, struct employee_t* employees, const char* const employee_string, const char* const update_hours_str)
{
    if (dbhdr->count == 0) { printf("Trying to update an employee from an empty database\n"); return STATUS_ERROR; }

    char* name = NULL;
    char* addr = NULL;
    int hours = 0;
    if (parse_employee_string(&name, &addr, &hours, employee_string) == STATUS_ERROR) { free_parse_employee_string(&name, &addr); return STATUS_ERROR2; }

    if (!update_hours_str) { printf("Update hours value not provided\n"); free_parse_employee_string(&name, &addr); return STATUS_ERROR2; }
    const unsigned int new_hours = atoi(update_hours_str);
    if (!new_hours && strcmp(update_hours_str, "0")) { printf("ATOI failed\n"); free_parse_employee_string(&name, &addr); return STATUS_ERROR2; }

    bool found = false;
    for (int i = 0; i < dbhdr->count; i++)
    {
        if (!strcmp(name, employees[i].name) && !strcmp(addr, employees[i].address) && hours == employees[i].hours) {
            employees[i].hours = new_hours;
            found = true;
        }
    }

    free_parse_employee_string(&name, &addr);

    if (!found) { printf("Employee does not exist in the database\n"); return STATUS_ERROR; }

    return STATUS_SUCCESS;
}

int compare_employees(const void* a, const void* b)
{
    const struct employee_t* emp1 = (const struct employee_t*) a;
    const struct employee_t* emp2 = (const struct employee_t*) b;

    return strcmp(emp1->name, emp2->name);
}

int output_file(int* fd, struct dbheader_t* const dbhdr, struct employee_t* const employees, const char* const filepath)
{
    if (*fd < 0) { printf("Invalid file descriptor received by the user\n"); return STATUS_ERROR; }

    const int dbhdr_cnt = dbhdr->count; // we have to keep a copy of count in host form
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * dbhdr_cnt);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);

    // creating a tmp file which I will rename to the original after I remove it
    const char* const tmp_filename = "___tmp_db_file___";
    int tmp_fd = open(tmp_filename, O_WRONLY | O_CREAT, 0644);
    if (tmp_fd == -1) { fprintf(stderr, "Unable to create temporary file %s: %s\n", filepath, strerror(errno)); return STATUS_ERROR; }

    ssize_t bytes_written = write(tmp_fd, dbhdr, sizeof(struct dbheader_t));
    if (bytes_written == -1) { perror("Failed to write database header into output_file(database)"); return STATUS_ERROR; }
    if (bytes_written != sizeof(struct dbheader_t)) { fprintf(stderr, "Partial write: Expected %zu bytes from dbhdr, but wrote %zd bytes into output_file(database)\n", sizeof(struct dbheader_t), bytes_written); return STATUS_ERROR; }

    qsort(employees, dbhdr_cnt, sizeof(struct employee_t), compare_employees);

    for (int i = 0; i < dbhdr_cnt; i++)
    {
        employees[i].hours = htonl(employees[i].hours);

        bytes_written = write(tmp_fd, &employees[i], sizeof(struct employee_t));
        if (bytes_written == -1) { perror("Failed to write employees into output_file(database)"); return STATUS_ERROR; }
        if (bytes_written != sizeof(struct employee_t)) { fprintf(stderr, "Partial write: Expected %zu bytes from employees, but wrote %zd bytes into output_file(database)\n", sizeof(struct employee_t), bytes_written); return STATUS_ERROR; }
    
        employees[i].hours = ntohl(employees[i].hours);
    }

    close(tmp_fd);
    close(*fd);
    // deleting the old file and renaming the tmp to old file name
    if (unlink(filepath) == -1) { fprintf(stderr, "Unable to delete old file %s to replace with new one %s: %s\n", filepath, tmp_filename, strerror(errno)); return STATUS_ERROR2; }
    if (rename(tmp_filename, filepath) == -1) { fprintf(stderr, "Unable to rename %s to %s: %s\n", tmp_filename, filepath, strerror(errno)); return STATUS_ERROR2; }

    *fd = open(filepath, O_RDWR, 0644);
    if (*fd == -1) { fprintf(stderr, "Unable to open again %s: %s\n", filepath, strerror(errno)); return STATUS_ERROR2; }

    dbhdr->magic = ntohl(dbhdr->magic);
    dbhdr->filesize = ntohl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * dbhdr_cnt);
    dbhdr->count = ntohs(dbhdr->count);
    dbhdr->version = ntohs(dbhdr->version);

    return STATUS_SUCCESS;
}