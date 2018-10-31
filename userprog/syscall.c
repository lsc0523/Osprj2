#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

	
void syscall_init (void) 
{
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
void check_vaddr(void* esp)
{
	if(is_kernel_vaddr(esp))
		exit(-1);
}

	static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	//printf("syscall : %d\n",*(uint32_t *)(f->esp));
	/*printf("address : %10X\n\n",f->esp);
	printf("f->esp+4 is %d\n\n",*(uint32_t*)(f->esp +4));
	printf("f->esp+8 is %d\n\n",*(uint32_t*)(f->esp+8));
	printf("f->esp+12 is %d\n\n",*(uint32_t*)(f->esp+12));
	printf("f->esp+12 is %d\n\n",*(uint32_t*)(f->esp+16));
	printf("f->esp+12 is %d\n\n",*(uint32_t*)(f->esp+20));*/

	if(is_kernel_vaddr(f->esp))
		exit(-1);
	//hex_dump(f->esp,f->esp,100,1);
	int *temp=f->esp;
	struct thread* now=thread_current();
	switch(*(uint32_t *)(f->esp))
	{
		case SYS_HALT:
			halt();
			break;
		case SYS_EXIT:
			check_vaddr(f->esp+4);
			now->parent->exit_flag=temp[1];
			exit(*(uint32_t *)(f->esp +4));
			break;
		case SYS_EXEC://2
			check_vaddr(f->esp+4);
			f->eax=exec((const char*)*(uint32_t*)(f->esp +4));
			break;
		case SYS_WAIT:
			check_vaddr(f->esp+4);
			f->eax=wait( (pid_t)*(uint32_t *)(f->esp + 4));
			break;
		case SYS_CREATE:
			check_vaddr(f->esp+4);
			check_vaddr(f->esp+8);
			f->eax=create((const char*)*(uint32_t*)(f->esp+4),(unsigned)*(uint32_t*)(f->esp+8));
			break;
		case SYS_REMOVE:
			check_vaddr(f->esp+4);
			f->eax=remove((const char*)*(uint32_t*)(f->esp+4));
			break;
		case SYS_OPEN:
			check_vaddr(f->esp+4);
			f->eax=open((const char*)*(uint32_t*)(f->esp+4));
			break;
		case SYS_FILESIZE:
			check_vaddr(f->esp+4);
			f->eax=filesize((int)*(uint32_t*)(f->esp+4));
			break;
		case SYS_READ:
			check_vaddr(f->esp+20);
			check_vaddr(f->esp+24);
			check_vaddr(f->esp+28);
			f->eax=read((int)*(uint32_t *)(f->esp + 20), (void *)*(uint32_t *)(f->esp + 24), (unsigned)*((uint32_t *)(f->esp + 28)));
			break;
		case SYS_WRITE://9
			check_vaddr(f->esp+4);
			check_vaddr(f->esp+8);
			check_vaddr(f->esp+12);
			f->eax=write((int)*(uint32_t *)(f->esp+4), (void *)*(uint32_t *)(f->esp + 8 ), (unsigned)*((uint32_t *)(f->esp +12)));
			break;
		case SYS_SEEK:
			check_vaddr(f->esp+4);
			check_vaddr(f->esp+8);
			seek((int)*(uint32_t*)(f->esp+4),(unsigned)*(uint32_t*)(f->esp+8));
			break;
		case SYS_TELL:
			check_vaddr(f->esp+4);
			f->eax=tell((int)*(uint32_t*)(f->esp+4));
			break;
		case SYS_CLOSE:
			check_vaddr(f->esp+4);
			close((int)*(uint32_t*)(f->esp+4));
			break;
		case SYS_PIBO:
			check_vaddr(f->esp+4);
			f->eax=pibonacci((int)*(uint32_t*)(f->esp+4));
			break;
		case SYS_SUM:
			check_vaddr(f->esp+4);
			check_vaddr(f->esp+8);
			check_vaddr(f->esp+12);
			check_vaddr(f->esp+16);
			//f->eax=pibonacci((int)*(uint32_t *)(f->esp+4));
			f->eax=sum_of_four_integers((int)*(uint32_t *)(f->esp+4),(int)*(uint32_t *)(f->esp+8),(int)*(uint32_t *)(f->esp+12),(int)*(uint32_t *)(f->esp+16));
			break;

	}
	//printf ("system call!\n");
	//thread_exit ();
}

void halt(void)
{
	shutdown_power_off();
}

void exit(int status)
{
	struct thread* now=thread_current();
	list_remove(&(now->child_elem));/*
	printf("%s: exit(%d)\n", thread_name(),status);
	now->parent->child_status=THREAD_DYING;
	thread_exit();*/
	if(now->parent!=NULL)
	{
		now->parent->child_status=THREAD_DYING;
		now->parent->waiting=false;
	}
	printf("%s: exit(%d)\n",thread_name(),status);
	thread_exit();

}

pid_t exec(const char *cmd)
{
//	printf("********* syscall : %s *******\n",cmd);
	return process_execute(cmd);
}

int wait(pid_t pid)
{
	return process_wait(pid);

}

int read(int fd, void* buffer, unsigned size)
{
	int i=0;
	check_vaddr(buffer);
	if(fd==0){
		for (i=0;i<size;i++){
			if(*(uint8_t *)(buffer+i) = input_getc()){
					break;
			}		
		}
		if(i!=size)
			return -1;
	}
	
	struct thread* now_t=thread_current();
	if(now_t->FD[fd]==NULL)
		return -1;
	return file_read(now_t->FD[fd],buffer,size);

}

int write(int fd,const void *buffer, unsigned size)
{
	check_vaddr(buffer);
	if(fd==1){
	
		putbuf(buffer,size);
		return size;
	}
	struct thread* now_t=thread_current();
	if(now_t->FD[fd]==NULL)
		return 0;
	return file_write(now_t->FD[fd],buffer,size);
}

int pibonacci(int n){
	if(n==0)
		return 0;
	if(n==1)
		return 1;
	if(n==2)
		return 1;
	
	int i,n1=1,n2=1,temp;
	for(i=0;i<n-2;i++){
		temp = n1+n2;
		n1 = n2;
		n2 = temp;
	}

	return temp;
}

int sum_of_four_integers(int a,int b,int c,int d){
	return a+b+c+d;
}

bool create(const char *file, unsigned initial_size)
{
	if(file==NULL)
		exit(-1);
	check_vaddr(file);
	if(filesys_create(file,initial_size))
		return true;
	else
		return false;
}
bool remove(const char *file)
{
	if(file==NULL)
		exit(-1);
	if(filesys_remove(file))
		return true;
	else
		return false;
}
int open(const char* file)
{
	if(file==NULL)
		exit(-1);
	check_vaddr(file);
	struct file* ret=filesys_open(file);
	if(ret==NULL){//could not open
		//printf("없어ㅠㅠ\n");
		return -1;
	}
	struct thread* now_t=thread_current();
	int i;
	for(i=3;i<128;i++)
	{
		if(now_t->FD[i]==NULL){
			now_t->FD[i]=ret;
			return i;
		}
	}
	return -1;

}
int filesize(int fd)
{
	struct thread* now_t=thread_current();
	if(now_t->FD[fd]==NULL)
		exit(-1);
      	return file_length(now_t->FD[fd]);
}
void seek(int fd, unsigned position)
{
	struct thread* now_t=thread_current();
	if(now_t->FD[fd]==NULL)
		exit(-1);
	file_seek(now_t->FD[fd],position);

}
unsigned tell(int fd)
{
	struct thread* now_t=thread_current();
	if(now_t->FD[fd]==NULL)
		exit(-1);
	return file_tell(now_t->FD[fd]);
}
void close(int fd)
{
	struct thread* now_t=thread_current();
	if(now_t->FD[fd]==NULL)
		exit(-1);
	return file_close(now_t->FD[fd]);
}

