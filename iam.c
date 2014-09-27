#define __LIBRARY__
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

_syscall1(int,iam,const char*,name)

int main(int argc,char * args[]){
	if(argc > 1){
		if(iam(args[1]) < 0){
			printf("SystemCall Exception!\n");
			return -1;
		}
	}else{
		printf("Input Exception!\n");
		return -1;
	}
	return 0;
}