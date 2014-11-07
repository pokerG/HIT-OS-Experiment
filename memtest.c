#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#define CMD_LIMIT 10 /*the length of cmd */
#define THREAD_LIMIT 10 /*the max amount of thread */
#define TEST_LENGTH 0x00100000	/*the test memory size*/

typedef struct thread_args{
	char * start_addr;
	char * end_addr;
	long tested;
	int finished;
	int killed;
}thread_args;

char testdata[5] = {0,0xFF,0x55,0xAA,0};
char * mem; //memory use in test	
int tms; /* the number of times*/
int tds; /* the number of thread*/
pthread_t threads[THREAD_LIMIT];
thread_args td_args[THREAD_LIMIT] = {};

void memTest(void *args){
	thread_args * arg;
	arg = (thread_args *)args;
	srand((int)(arg->start_addr) % tds);
	testdata[4] = (char)rand();
	int i,j,k;
	char * addr = arg->start_addr;
	for(i = 0; i < arg->end_addr - arg->start_addr + 1;i++){
		for(j = 0; j < tms;j++){
			for(k = 0; k < 5;k++){
				if(arg->killed == 1){
					pthread_exit((void *)1);
				}
				(*addr) = testdata[k];
				if((*addr) != testdata[k]){
					pthread_exit((void *)-1);
				}
			}
		}
		arg->tested++;
		addr++;
	}
	arg->finished = 1;
	pthread_exit((void *)0);
}

int times(){
	int i;
	scanf("%d",&i);
	if(i <= 0){
		printf("Bad argument or not all threads has exited.\n");
		return -1;
	}
	tms = i;
	printf("Each byte will be tested by %d times\n",tms);
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
	printf("Will use %d threads\n",tds);
	
	td_args[tds - 1].end_addr += (TEST_LENGTH % tds);
	return 0;
}


int go(){
	int i;
	init();
	for(i = 0; i < tds;i++){
		pthread_create(&threads[i],NULL,(void *)memTest,(void *)&td_args[i]);
	}
} 

int status(){
	int i;
	for(i = 0; i < tds; i++){
		if(td_args[i].finished || td_args[i].killed)
			printf("Thread %d has exited.(%p-%p,%ld/%ld\n",i,td_args[i].start_addr,td_args[i].end_addr,td_args[i].tested,td_args[i].end_addr - td_args[i].start_addr + 1);
		else
			printf("Thread %d is running.(%p-%p,%ld/%ld\n",i,td_args[i].start_addr,td_args[i].end_addr,td_args[i].tested,td_args[i].end_addr - td_args[i].start_addr + 1);
	}
}

int aborts(){
	int i = 0;
	for(i = 0; i < tds;i++){
		td_args[i].killed = 1;
	}
}

int exits(){
	int i;
	for(i = 0; i < tds;i++){
		td_args[i].killed = 1;
	}
	for(i = 0; i < tds; i++){
		pthread_join(threads[i],NULL);
	}
}



void init(){
	int i;
	char * tmpstart = mem;
	int size = TEST_LENGTH / tds;
	for(i = 0;i < tds;i++){
		td_args[i].start_addr = tmpstart;
		td_args[i].end_addr = tmpstart + size - 1;
		td_args[i].tested = 0;
		td_args[i].finished = 0;
		td_args[i].killed = 0;
		tmpstart = td_args[i].end_addr + 1; 		
	}

}

int main(){
	char cmd[CMD_LIMIT];
	tds = 5;	//default value
	tms = 2;	//default value	
	init();
	mem = (char *)malloc(TEST_LENGTH);
	if(!mem){
		printf("Error:Memory is not enough!\n");
		exit(0);
	}
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
			aborts();
		}else if(!strcmp(cmd,"exit")){
			exits();
			break;
		}else
			printf("Bad command!\n");
	}
	return 0;
}

