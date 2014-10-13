/*
	linux 0.11/kernel/who.c
	(C) 2014 LeiGuang Hao (pokerface)
	
*/

#include <errno.h>
#include <asm/segment.h>

char kName[25] = {0}; //store name in kernel

int sys_iam(const char* name){
	int i = 0;
	while(get_fs_byte(name + i) != '\0'){
		i++;
		if(i > 23){
			return -EINVAL;
		}
	}
	printk("%d\n",i);
	int count = i;
	for(;i >= 0; i--){
		kName[i] = get_fs_byte(name + i);	
	}
	return count;
}

int sys_whoami(char* name,unsigned int size){
	int i = 0;
	while(kName[i] != '\0'){
		i++;
	}
	if(size < i){
		return -EINVAL;
	}
	i = 0;
	for(;kName[i] != '\0';i++){
		put_fs_byte(kName[i],name + i);
	}
	//printk("%s\n",name);

	return i;
}
