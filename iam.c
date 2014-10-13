#define __LIBRARY__
#include <unistd.h>
#include <stdio.h>

_syscall1(int,iam,const char*,name)

int main(int argc,char* argv[]){
	if(argc > 1){
		if(iam(argv[1]) < 0){
			printf("SysCall Exception!\n");
			return -1;
		}
	}else{
		printf("Please input name!\n");
		return -1;
	}
	return 0;
}