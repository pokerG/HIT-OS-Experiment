/*
	linux 0.11/kernel/who.c
	(C) 2014 LeiGuang Hao (pokerface)
	
*/

#define __LIBRARY__
#include <unistd.h>
#include <errno.h>
#include <asm/segment.h>
	
char sName[30] = {0};

int sys_iam(const char * name){
	int i = 0;
	while(get_fs_byte(name + i) != '\0')
		i++;
	if(i > 23){
		return -EINVAL;
	}
	printk("%d\n",i);
	int count = i;
	for(;i >= 0; i--)
		sName[i] = get_fs_byte(name + i);

	return count;
}

int sys_whoami(char * name,unsigned int size){
	int  i = 0;
	while(sName[i] != '\0')
		i++;
	if(size < i){
		return -EINVAL;
	}
	i = 0;
	while(sName[i] != '\0'){
		put_fs_byte(sName[i], name + i);
		i++;
	}
	
	return i;
}