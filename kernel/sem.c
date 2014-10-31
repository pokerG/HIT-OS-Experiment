#include <unistd.h>
#include <errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <asm/segment.h>

sem_t* semaphore[20];
int sem_num = 0; //thera are some problems

// char sem[SEM_NUM][SEM_NAME_LIMIT];

sem_t* sys_sem_open(const char *name,unsigned int value){
	sem_t *newsem;
	cli();
	newsem = (sem_t *)get_free_page();
	if(!newsem){
		return NULL;
	}
	int i = 0;
	while(get_fs_byte(name + i) != '\0'){
		i++;
		if(i > SEM_NAME_LIMIT){
			errno = EINVAL;
			return NULL;
		}
	}
	for(;i >= 0; i--){
		newsem->name[i] = get_fs_byte(name + i);	
	}
	newsem->value = value;
	// newsem->wait_task = newsem + sizeof(char) * SEM_NAME_LIMIT + sizeof(int) ;
	// newsem->wait_task += sizeof(int);
	(newsem->wait_task).front = 0;
	(newsem->wait_task).rear = 0;
	semaphore[sem_num] = newsem;
	sem_num ++;
	sti();
	return newsem;
}

int sys_sem_wait(sem_t * sem){
	if(!sem){
		return -EINVAL;
	}
	cli();
	sem->value --;
	if(sem->value < 0 ){
		(sem->wait_task).task[(sem->wait_task).rear] = current;
		(sem->wait_task).rear = ((sem->wait_task).rear + 1) % QUEUE_LIMIT	 ;
		current->state = TASK_UNINTERRUPTIBLE;  
        schedule();  
	}
	sti();
	return 0;
}

int sys_sem_post(sem_t * sem){
	if(!sem){
		return -EINVAL;
	}
	cli();
	sem->value ++;
	if(sem->value <= 0){
		wake_up((sem->wait_task).task[(sem->wait_task).front]);
		(sem->wait_task).front = ((sem->wait_task).front + 1) % QUEUE_LIMIT;
	}
	sti();
	return 0;
}

int sys_sem_unlink(const char *name){
	cli();
	int i; 
	for(i = 0; i < sem_num;i++){
		if(semaphore[i] != NULL && strcmp(semaphore[i]->name,name) == 0){
			free_page((long)semaphore[i]);
			break;
		}
	}
	sti();
	return i == sem_num ? -1 : 0; 
}