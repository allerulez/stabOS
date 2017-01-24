#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	//f->esp holds the adress of the first syscall argument
	printf("system call\n");
  int *sp = f->esp;
  //int *syscall_no = (int *) sp;
  switch((int) *sp) {
  	case SYS_HALT:
  		halt();
  		break;
  	case SYS_CREATE:
  		create((char*) *(sp + 4), (int) *(sp + 8));
  		break;
		case SYS_OPEN:
			open((char*) *(sp + 4));

			break;
		case SYS_CLOSE:
			close((int) *(sp + 4));
			break;
		case SYS_READ:
			read((int) *(sp + 4), (char *) *(sp + 8), (unsigned) *(sp + 12));
			break;
		case SYS_WRITE:
			write((int) *(sp + 4), (void *) *(sp + 8), (unsigned) *(sp + 12));
			break;
		case SYS_EXIT:
			exit((int) *(sp + 4));
			break;
    }
  	thread_exit ();
}

void halt(void) {
	power_off();
}

bool create(const char *file, unsigned initial_size) {
	return filesys_create(file, initial_size);
}

int open(const char *file) {
	struct file * cur_file = (struct file *) filesys_open(file);
	struct thread * cur_thread = thread_current();
	if(file != NULL) {
		cur_thread->files[cur_thread->file_no] = cur_file;
		cur_thread->file_no++;
		return cur_thread->file_no + 1;
	}else {
		return -1;
	}
}

void close(int fd) {
	struct thread * cur_thread = thread_current();
	file_close(cur_thread->files[fd-2]);
	if(fd < cur_thread->file_no+2) {
		int i;
		for (i = fd-2; i <(cur_thread->file_no); i++) {
			cur_thread->files[i] = cur_thread->files[i+1];
		}
		cur_thread->file_no--;
	}

}

int read(int fd, char *buffer, unsigned size) {
	struct thread * cur_thread = thread_current();
	if(fd == 0) {
		int i;
		for(i = 0; i<(int) size; i++) {
			*buffer = (input_getc());
			buffer++;
		}
		return sizeof(input_getc())*(int)size;
	} else if(fd==1) {
		return -1;
	}
	if(fd < cur_thread->file_no+2) {
		return file_read(cur_thread->files[fd-2], buffer, (off_t) size);
	}
	return -1;
}

int write(int fd, const void *buffer, unsigned size) {
	struct thread * cur_thread = thread_current();
	int retSize = size;
	if(fd==0) {
		return -1;
	}else if(fd==1) {
		while(size > 200) {
			putbuf(buffer, (size_t) 200);
			size-= 200;
		}
		putbuf(buffer, (size_t) size);
		return (int) retSize;
	} else {
		if(fd < cur_thread->file_no+2) {
			return file_write(cur_thread->files[fd-2], buffer, (off_t) size);
		}
	}
	return -1;
}

void exit(int status) {
	status++;
	status--;
	struct thread * cur_thread = thread_current();
	int i;
	for (i = 0; i<cur_thread->file_no; i++) {
		//free(cur_thread->files[i]);
	}
	thread_exit();
}