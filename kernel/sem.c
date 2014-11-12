#include <unistd.h>
#include <errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <asm/segment.h>

#define SEM_NUM 20 // the max amount of sem
sem_t* semaphore[SEM_NUM];

sem_t* sys_sem_open(const char *name,unsigned int value){
	sem_t *newsem;
	char sname[SEM_NAME_LIMIT];
	int i;
	for(i = 0; (sname[i] = get_fs_byte(name + i)) != '\0' && i < SEM_NAME_LIMIT;i++);
	if(i >= SEM_NAME_LIMIT){
			return NULL;
	}
	for(i = 0; i < SEM_NUM; i++){
		if(semaphore[i] != NULL && strcmp(semaphore[i]->name,sname) == 0){
			return semaphore[i];  //already have this sem
		}
	}
	newsem = (sem_t *)get_free_page(); // the idea from fork and exit
	if(!newsem) return NULL;
	strcpy(newsem->name,sname);
	newsem->value = value;
	(newsem->wait_task).front = 0;
	(newsem->wait_task).rear = 0;
	for(i = 0 ; i < SEM_NUM; i++){
		if(semaphore[i] == NULL){
			semaphore[i] = newsem;
			break;
		}
	}
	return i == SEM_NUM ? NULL:newsem;
}

int sys_sem_wait(sem_t * sem){
	if(!sem) return -EINVAL;
	cli();
	sem->value --;
	if(sem->value < 0 ){
		if(((sem->wait_task).rear + 1) % QUEUE_LIMIT != (sem->wait_task).front){
			(sem->wait_task).task[(sem->wait_task).rear] = current;
			(sem->wait_task).rear = ((sem->wait_task).rear + 1) % QUEUE_LIMIT;
			current->state = TASK_UNINTERRUPTIBLE;
			schedule();
		} else{
			sti();
			return -1;
		}	
	}
	sti();
	return 0;
}

int sys_sem_post(sem_t * sem){
	if(!sem) return -EINVAL;
	cli();
	sem->value ++;
	if(sem->value <= 0){
		(sem->wait_task).task[(sem->wait_task).front]->state = TASK_RUNNING;
		(sem->wait_task).front = ((sem->wait_task).front + 1) % QUEUE_LIMIT;
	}
	sti();
	return 0;
}

int sys_sem_unlink(const char *name){
	int i; 
	char sname[SEM_NAME_LIMIT];
	for(i = 0; (sname[i] = get_fs_byte(name + i)) != '\0' && i < SEM_NAME_LIMIT;i++);
	for(i = 0; i < SEM_NUM;i++){
		if(semaphore[i] != NULL && strcmp(semaphore[i]->name,sname) == 0){
			free_page((long)semaphore[i]);
			break;
		}
	}
	return i == SEM_NUM ? -1 : 0; 
}