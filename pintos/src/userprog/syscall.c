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
  int *sp = f->esp;
  //int *syscall_no = (int *) sp;
  switch((int) *sp) {
  	case SYS_HALT:
  		halt();
  		break;
  	case SYS_CREATE:
  		f->eax = create(*((char**) (sp + 1)), (int) *(sp + 2));
  		break;
		case SYS_OPEN:
			f->eax = open(*((char**) (sp + 1)));
			break;
		case SYS_CLOSE:
			close((int) *(sp + 1));
			break;
		case SYS_READ:
			f->eax = read((int) *(sp + 1), *((char**) (sp + 2)), (unsigned) *(sp + 3));
			break;
		case SYS_WRITE:
			f->eax = write((int) *(sp + 1), (void *) *(sp + 2), (unsigned) *(sp + 3));
			//printf("wrote %i bytes to file with FD: %i.", bytes_written, (int) *(sp+1));
			break;
		case SYS_EXIT:
			exit((int) *(sp + 1));
			break;
		case SYS_EXEC:
			wait(*((char**) (sp + 1)));
			break;
    }
}

void halt(void) {
	printf("halt system call\n");
	power_off();
}

bool create(const char *file, unsigned initial_size) {
	//printf("create system call\n");
	bool thisbool = filesys_create(file, (off_t) initial_size);
	return thisbool;
}

int open(const char *file) {
	//printf("open system call\n");
	struct file * cur_file = (struct file *) filesys_open(file);
	struct thread * cur_thread = thread_current();
	if (cur_file == NULL) return -1;
	if(cur_thread->files[cur_thread->file_no] == NULL && cur_thread->file_no < 127) {
		cur_thread->files[cur_thread->file_no] = cur_file;
		cur_thread->file_no++;
		cur_thread->files_open++;
		return cur_thread->file_no + 1;
	} else if (cur_thread->file_no >= 127) {
		int i;
		for (i = 0; i < 127; i++) {
			if (cur_thread->files[i] == NULL) {
				cur_thread->files[i] = cur_file;
				cur_thread->files_open++;
				return i + 2;
			}
		}
	}
	return -1;
}

void close(int fd) {
	//printf("close system call\n");
	struct thread * cur_thread = thread_current();
	if (cur_thread->file_no == fd - 2) cur_thread->file_no--;
	file_close(cur_thread->files[fd - 2]);
	cur_thread->files[fd-2] = NULL;
}

int read(int fd, char *buffer, unsigned size) {
	//printf("read system call\n");
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
	} else if(fd==1) {
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
  struct thread * cur_thread = thread_current();
  lock_init(&cur_thread->l);
  lock_acquire(&cur_thread->l);
  if(cur_thread->parent_pair->parent == NULL){
    free(cur_thread->parent_pair);
  }
  for (e = list_begin (&cur_thread->children); e != list_end (&cur_thread->children);
  e = list_next (e)){
    struct pair * child_pair = list_entry (e, struct pair, pair_elem);
    child_pair->state--;
    if(child_pair->state == 0) free(child_pair);
  }
  lock_release(cur_thread->&l);
  thread_exit();
}

pid_t exec(const char* cmd_line) {
  struct * cur_thread = thread_current();

  //sema_down(&cur_thread->wait);
  process_execute(cmd_line);
  int child_id = list_entry(list_end(&cur_thread->children), struct pair, pair_elem)->child->tid;
  //make sure to return id properly
  if (child_id == TID_ERROR) return -1;
  {
    return child_id;
  }
}

int wait(pid_t pid) {
  struct * cur_thread = thread_current();
  int status = -1;
  lock_init(&cur_thread->l);
  lock_acquire(&cur_thread->l);
  for (e = list_begin (&cur_thread->children); e != list_end (&cur_thread->children);
  e = list_next (e))
  {
    struct pair * child_pair = list_entry (e, struct pair, pair_elem);
    if(child_pair->child->tid == pid) {
      status = child_pair->exit_status_child;
      if(child_pair->exit_status_child != NULL) {
        return status;
      }
      break;
    }
  }
  sema_init(&cur_thread->wait,0);
  cur_thread->parent_wait = true;
  sema_down(&cur_thread->wait);
  lock_release(cur_thread->&l);
  return status;
}
