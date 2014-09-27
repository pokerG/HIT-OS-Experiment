#define __LIBRARY__
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

_syscall2(int,whoami,char*,name,unsigned int,size)

int main(){
	int count;
	char name[30] = {0};
	count = whoami(name,30);
	if(count < 0){
		printf("SystemCall Exception!\n");
		return -1;
	}else{
		printf("%s\n",name);
	}
	return 0;
}