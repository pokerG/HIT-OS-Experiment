#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CMD_LIMIT 10 /*the length of cmd */
#define THREAD_LIMIT 10 /*the max amount of thread */
int tms; /* the number of times*/
int tds; /* the number of thread*/

int times(){
	int i;
	scanf("%d",&i);
	if(i <= 0){
		printf("Bad argument or not all threads has exited.\n");
		return -1;
	}
	tms = i;
	return 0;
}

int thread(){
	int i;
	scanf("%d",&i);
	if(i > THREAD_LIMIT || i <= 0){
		printf("Bad argument or not all threads has exited.\n");
		return -1;
	}
	tds = i;
	return 0;
}


int 
int main(){
	char cmd[CMD_LIMIT]
	while(1){
		printf(">>>");
		scanf("%s",&cmd);
		if(!strcmp(cmd,"times")){
			times();
		}else if(!strcmp(cmd,"thread")){
			thread();
		}else if(!strcmp(cmd,"go")){
			go();
		}else if(!strcmp(cmd,"status")){
			status();
		}else if(!strcmp(cmd,"abort")){
			_abort();
		}else if(!strcmp(cmd,"exit")){
			_exit();
		}else
			printf("Bad command!\n");
	}
}