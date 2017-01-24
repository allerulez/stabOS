#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	//f->esp holds the adress of the first syscall argument
    int syscall_no = (int *) f->esp;
    switch(syscall_no) {
    	case SYS_HALT:
    		halt();
    		break;
    	case SYS_CREATE:
    		create((int *) f->esp + 1, (int *) f->esp + 2);
    		break;
		case SYS_OPEN:
			open((char *) f->esp + 1);
    }
  	thread_exit ();
}

void halt(void) {
	power_off();
}