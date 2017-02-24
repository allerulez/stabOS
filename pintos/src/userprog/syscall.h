#include <stdbool.h>
#include "process.h"
#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void halt(void);
bool create(const char *file, unsigned initial_size);
int open(const char *file);
void close(int fd);
int read(int fd, char *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void exit(int status);
tid_t exec(const char* cmd_line);
int wait(tid_t pid);

#endif /* userprog/syscall.h */
