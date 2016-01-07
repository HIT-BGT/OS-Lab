#define   __LIBRARY__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 

#define EMPTY_SEM_NAME "/empty_sem_name"
#define EMPTY_SEM_VALUE 10
#define FULL_SEM_NAME "/full_sem_name"
#define FULL_SEM_VALUE 0
#define MUTEX_SEM_NAME "/mutex_sem_name"
#define MUTEX_SEM_VALUE 1
#define N 5	/* The number of consumer processes */
#define M 501

#define KEYNUM 10 /* The first param of shmget() */
#define PAGE 4096 /* The second param of shmget() */

_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name); 
_syscall3(int,shmget,int,key,size_t,size,int,shmflag);  
_syscall3(void *,shmat,int,shmid,const void*, shmaddr,int,shmflag);  

/* Declaration of producer process */
void produce();

sem_t *empty_sem;
sem_t *full_sem;
sem_t *mutex_sem;
int trueaddress, *offsetaddress;

int main(int argc, char const *argv[])
{
	int key = 10;
	int i,fail_flag=0;
	freopen("data.out","w",stdout);
	empty_sem = sem_open(EMPTY_SEM_NAME, 10);
	full_sem = sem_open(FULL_SEM_NAME, 0);
	mutex_sem = sem_open(MUTEX_SEM_NAME, 1);
	trueaddress = shmget(key,4096,0);
	if (trueaddress==-1){
		printf("Error encountered when open/creating share memory.\n");
		fail_flag = 1;	/* Set the fail flag */
	}

	if (fail_flag==0) {
		offsetaddress = (int*)shmat(trueaddress,NULL,0);
		produce();
	}
	sem_unlink(EMPTY_SEM_NAME);
	sem_unlink(FULL_SEM_NAME);
	sem_unlink(MUTEX_SEM_NAME);
	return 0;
}

/* Definition of producer process */
void produce(){
	int i;
	for (i=0; i<=M; i++){
		sem_wait(empty_sem);	/* P(Empty) */
		sem_wait(mutex_sem);	/* P(Mutex) */
		offsetaddress[i%10] = i;
		sem_post(mutex_sem);	/* V(Mutex) */
		sem_post(full_sem);		/* V(Full) */
	}
}