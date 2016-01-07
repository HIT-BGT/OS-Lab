#include <fcntl.h>	// To use O_* procedures
#include <sys/types.h> // To use "pid_t" type
#include <sys/stat.h> // To use permission(S_*) values
#include <semaphore.h>
#include <unistd.h>	// To use fork()
#include <stdio.h>	// To use printf()
#include <stdlib.h>	// To use freopen()
#include <wait.h>	// To use wait()
#include <sys/ipc.h> // To use IPC_CREAT and ftok()
#include <sys/shm.h> // To use shmget() and shmat()

#define EMPTY_SEM_NAME "/empty_sem_name"
#define EMPTY_SEM_VALUE 10
#define FULL_SEM_NAME "/full_sem_name"
#define FULL_SEM_VALUE 0
#define MUTEX_SEM_NAME "/mutex_sem_name"
#define MUTEX_SEM_VALUE 1
#define N 5	//The number of consumer processes
#define M 501

#define KEYNUM 10 // The first param of shmget()
#define PAGE 4096 // The second param of shmget()

// Declaration of producer process
void produce();

sem_t *empty_sem;
sem_t *full_sem;
sem_t *mutex_sem;
int fd; // The file description
int out_fd;	// Print result to this file
int in = 0;
int out = 0; 
int trueaddress, *offsetaddress;

int main(int argc, char const *argv[])
{
	key_t key = ftok(".", 4);
	int i,fail_flag=0;
	empty_sem = sem_open(EMPTY_SEM_NAME, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, EMPTY_SEM_VALUE); // mode = 00700|00070|00007
	full_sem = sem_open(FULL_SEM_NAME, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 
		FULL_SEM_VALUE); // mode = 00700|00070|00007
	mutex_sem = sem_open(MUTEX_SEM_NAME, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, 
		MUTEX_SEM_VALUE); // mode = 00700|00070|00007
	trueaddress = shmget(key,4096,IPC_CREAT|IPC_EXCL|0600);
	if (trueaddress==-1){
		printf("Error encountered when open/creating share memory.\n");
		fail_flag = 1;	// Set the fail flag
	}

	if (fail_flag==0) {
		offsetaddress = (int*)shmat(trueaddress,NULL,0);
		produce();
	}

	// for (i=0; i<N; i++) printf("Consumer process #%d PID: %d\n", i, con_pids[i]);
	sem_unlink(EMPTY_SEM_NAME);
	sem_unlink(FULL_SEM_NAME);
	sem_unlink(MUTEX_SEM_NAME);
	shmctl(trueaddress, IPC_RMID, NULL);
	return 0;
}

// Definition of producer process
void produce(){
	int i;
	for (i=0; i<=M; i++){
		sem_wait(empty_sem);	// P(Empty)
		sem_wait(mutex_sem);	// P(Mutex)
		offsetaddress[i%10] = i;
		sem_post(mutex_sem);	// V(Mutex)
		sem_post(full_sem);		// V(Full)
	}
}