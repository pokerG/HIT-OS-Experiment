#define   __LIBRARY__ 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#define BUFFER_SIZE 10
#define M 555 //写入的最大数据
#define N 5 //消费者数

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
	sem_t * empty, * full, * mutex;
	empty = sem_open("empty",O_CREAT,0666,BUFFER_SIZE);
	full = sem_open("full",O_CREAT,0666,0);
	mutex = sem_open("mutex",O_CREAT,0666,1);
	int file = open("write.txt",O_RDWR|O_CREAT,0666);
	int in_pos = 0;
	int out_pos = 0;
	int tmpNum;
	writeBuffer(file,10,out_pos);	//out_pos shared by all consumer
	if(!fork()){
		for(i = 0; i < M; i++){
			sem_wait(empty);
			sem_wait(mutex);
			writeBuffer(file,in_pos,i);
			in_pos = (in_pos + 1) % BUFFER_SIZE;
			sem_post(mutex);
			sem_post(full);
		}
		exit(0);
	}

	for(j = 0; j < N;j++){
		if(!fork()){
			for(k = 0; k < M / N; k ++){
				sem_wait(full);
				sem_wait(mutex);
				readBuffer(file,10,&out_pos);	
				readBuffer(file,out_pos,&tmpNum);
				printf("%d thread print : %d\n",getpid(),tmpNum);
				fflush(stdout);  
				out_pos = (out_pos + 1) % BUFFER_SIZE;
				writeBuffer(file,10,out_pos);
				sem_post(mutex);
				sem_post(empty);
			}
			exit(0);
		}
	}
    while(-1 != wait(NULL));
	sem_unlink("empty");
	sem_unlink("full");
	sem_unlink("mutex");
	close(file);
	return 0;
}
