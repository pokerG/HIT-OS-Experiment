#define __LIBRARY__
#include <unistd.h>
#include <stdio.h>
#include <string.h>

_syscall2(int,whoami,const char*,name,unsigned int,size);

int main(){
	int count;
	char name[25] = {0};
	count = whoami(name,25);
	if(count <0){
		printf("SysCall Exception!\n");
		return -1;
	}else{
		printf("%s\n", name);
	}
	return 0;
}