#ifndef PTHREAD_H
#define PTHREAD_H
#define __LIBRARY__
#include <unistd.h>
int pthread_attr_init(pthread_attr_t *attr);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
void pthread_exit(void *value_ptr);
int pthread_join(pthread_t thread, void **value_ptr);
#endif


