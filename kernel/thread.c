#include <errno.h>
#include <unistd.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <signal.h>
#include <sys/wait.h>
#include <linux/tty.h>

extern int last_pid;
extern void tell_father(int);
extern int copy_mem(int,struct task_struct*);
extern void release(struct task_struct*);


int sys_tdattrinit(pthread_attr_t *attr){
	if(!attr)
		return -EINVAL;
	attr->state = 0;
	attr->stacksize = PAGE_SIZE;
	return 0;
}

int copy_thread(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	    struct task_struct *p;
    int i;
    struct file *f;

    p = (struct task_struct *) get_free_page();
    if (!p)
        return -EAGAIN;
    task[nr] = p;
    *p = *current;	/* NOTE! this doesn't copy the supervisor stack */
    p->state = TASK_UNINTERRUPTIBLE;
    p->pid = last_pid;
    p->father = current->pid;
    p->counter = p->priority;
    p->signal = 0;
    p->alarm = 0;
    p->leader = 0;		/* process leadership doesn't inherit */
    p->utime = p->stime = 0;
    p->cutime = p->cstime = 0;
    p->start_time = jiffies;
    p->tss.back_link = 0;
    p->tss.esp0 = PAGE_SIZE + (long) p;
    p->tss.ss0 = 0x10;
    p->tss.eip = ebx;    //first param
    p->tss.eflags = eflags;
    p->tss.eax = 0;
    p->tss.ecx = ecx;
    p->tss.edx = edx;
    p->tss.ebx = ebx;
    p->tss.esp = ecx;   //second param
    p->tss.ebp = ebp;
    p->tss.esi = esi;
    p->tss.edi = edi;
    p->tss.es = es & 0xffff;
    p->tss.cs = cs & 0xffff;
    p->tss.ss = ss & 0xffff;
    p->tss.ds = ds & 0xffff;
    p->tss.fs = fs & 0xffff;
    p->tss.gs = gs & 0xffff;
    //p->tss.ldt = _LDT(nr);
    p->tss.ldt = current->tss.ldt;
    p->tss.trace_bitmap = 0x80000000;
    if (last_task_used_math == current)
        __asm__("clts ; fnsave %0"::"m" (p->tss.i387));
    if (copy_mem(nr,p)) { 
     	task[nr] = NULL; 
     	free_page((long) p); 
     	return -EAGAIN; 
    }
    for (i=0; i<NR_OPEN;i++)
        if ((f=p->filp[i]))
            f->f_count++;
    if (current->pwd)
        current->pwd->i_count++;
    if (current->root)
        current->root->i_count++;
    if (current->executable)
        current->executable->i_count++;
    set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
    set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
    p->state = TASK_RUNNING;	/* do this last, just in case */
    return last_pid;
}

void sys_tdexit(void * value_ptr){
	current->state = TASK_ZOMBIE;
	current->exit_code = value_ptr;
	tell_father(current->father);
	schedule();
	return -1;
}

int sys_tdwait(pthread_t thread,void **value_ptr)
{	
	int flag, code;
	struct task_struct ** p;

	verify_area(*value_ptr,4);
repeat:
	flag=0;
	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p || *p == current)
			continue;
		if ((*p)->father != current->pid)
			continue;
		if (thread>0) {
			if ((*p)->pid != thread)
				continue;
		} else if (!thread) {
			if ((*p)->pgrp != current->pgrp)
				continue;
		} else if (thread != -1) {
			if ((*p)->pgrp != -thread)
				continue;
		}
		switch ((*p)->state) {
			case TASK_STOPPED:
				if (!WUNTRACED)
					continue;
				put_fs_long(0x7f,(unsigned long*)value_ptr);
				return (*p)->pid;
			case TASK_ZOMBIE:
				current->cutime += (*p)->utime;
				current->cstime += (*p)->stime;
				flag = (*p)->pid;
				code = (*p)->exit_code;
				release(*p);
				put_fs_long(code,(unsigned long*)value_ptr);
				return flag;
			default:
				flag=1;
				continue;
		}
	}
	if (flag) {
		if (WNOHANG)
			return 0;
		current->state=TASK_INTERRUPTIBLE;
		schedule();
		if (!(current->signal &= ~(1<<(SIGCHLD-1))))
			goto repeat;
		else
			return -EINTR;
	}
	return -ECHILD;
}