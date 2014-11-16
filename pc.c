#define   __LIBRARY__ 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#define BUFFER_SIZE 10
#define M 555 
#define N 5 

_syscall2(int,sem_open,const char*,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char*,name);

int writeBuffer(int fd,int in_pos,int num){
	lseek(fd,in_pos * sizeof(int),SEEK_SET);
	return write(fd,&num,sizeof(int));
}
int readBuffer(int fd,int out_pos,int* num){
	lseek(fd,out_pos * sizeof(int),SEEK_SET);
	return read(fd,num,sizeof(int));
}

int main(){
	int i,j,k;
	int fd;
	int in_pos = 0;
	int out_pos = 0;
	int tmpNum;
	fd = open("write.txt",O_RDWR|O_CREAT,0666);
	writeBuffer(fd,10,out_pos);	
	
	if(!fork()){
		sem_t   *empty, *full, *mutex;
		empty =(sem_t *)sem_open("empty",BUFFER_SIZE);
		full  = (sem_t *)sem_open("full",0);
   	 	mutex = (sem_t *)sem_open("mutex",1);
		for(i = 0; i < M; i++){
			sem_wait(empty);
			sem_wait(mutex);
			writeBuffer(fd,in_pos,i);
			in_pos = (in_pos + 1) % BUFFER_SIZE;
			sem_post(mutex);
			sem_post(full);
		}
		exit(0);
	}

	for(j = 0; j < N;j++){
		if(!fork()){
			sem_t   *empty, *full, *mutex;
			empty =(sem_t *)sem_open("empty",BUFFER_SIZE);
			full  = (sem_t *)sem_open("full",0);
   	 		mutex = (sem_t *)sem_open("mutex",1);
			for(k = 0; k < M / N; k ++){
				sem_wait(full);
				sem_wait(mutex);
				readBuffer(fd,10,&out_pos);	
				readBuffer(fd,out_pos,&tmpNum);
				printf("%d thread print : %d\n",getpid(),tmpNum);
				fflush(stdout);  
				out_pos = (out_pos + 1) % BUFFER_SIZE;
				writeBuffer(fd,10,out_pos);
				sem_post(mutex);
				sem_post(empty);
			}
			exit(0);
		}
	}
	while( -1 !=  wait(NULL) ); 
	sem_unlink("empty");
	sem_unlink("full");
	sem_unlink("mutex"); 
	close(fd);
	return 0;
}
