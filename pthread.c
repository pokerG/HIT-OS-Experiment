#define __LIBRARY__
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

_syscall1(int,tdattrinit,pthread_attr_t *,attr)
_syscall3(int,tdcreate,const pthread_attr_t *,attr,void *,start_routine,void *,arg)
_syscall1(void,tdexit,void *,value_ptr)
_syscall2(int,tdwait,pthread_t,thread,void **,value_ptr)

int pthread_attr_init(pthread_attr_t *attr){
	attr->state = 0;
	attr->stacksize = 4096;
	return 0;
}

int pthread_create(pthread_t *thread,const pthread_attr_t * attr,void *(*start_routine)(void *),void * arg){
	int id = 0;
	if(!thread) return EINVAL;	
	
	id = tdcreate(attr,start_routine,arg);
	if(id < 0) return errno;
	*thread = id;
	return 0;
}

void pthread_exit(void *value_ptr){
	tdexit(value_ptr);
}

int pthread_join(pthread_t thread,void **value_ptr){
	tdwait(thread,value_ptr);
	return 0;
}