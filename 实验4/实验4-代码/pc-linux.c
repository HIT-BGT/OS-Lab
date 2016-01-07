#define   __LIBRARY__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name);

#define EMPTY_SEM_NAME "/empty_sem_name"
#define EMPTY_SEM_VALUE 10
#define FULL_SEM_NAME "/full_sem_name"
#define FULL_SEM_VALUE 0
#define MUTEX_SEM_NAME "/mutex_sem_name"
#define MUTEX_SEM_VALUE 1
#define N 5
#define M 501

void produce();

void consume();

sem_t *empty_sem;
sem_t *full_sem;
sem_t *mutex_sem;
int fd;
/* int out_fd; */
int in = 0;
int out = 0; 

int main(int argc, char const *argv[])
{
	int i;
	pid_t pro_pid;
	pid_t con_pids[N];
	freopen("data.out","w",stdout);  
	empty_sem = sem_open(EMPTY_SEM_NAME, 10);
	full_sem = sem_open(FULL_SEM_NAME, 0);
	mutex_sem = sem_open(MUTEX_SEM_NAME, 1);
	fd = open("buffer", O_CREAT|O_TRUNC|O_RDWR, 0666);
	/* out_fd = open("result.c", O_CREAT|O_TRUNC|O_RDWR, 0666); */
	if (fd<0){
		printf("Failed to open buffer file!\n");
		return -1;
	}
	lseek(fd, 0, SEEK_SET);
	write(fd, (char*)(&out), sizeof(int));
	pro_pid = fork();
	if (pro_pid==0){
		produce();
		return 0;
	}else if(pro_pid<0){
		printf("Error encountered while creating a producer process!\n");
		return -1;
	}else{
		/*printf("Producer process PID: %d\n", pro_pid);*/
	}
	for (i=0; i<N; i++){
		con_pids[i] = fork();
		if (con_pids[i] == 0){
			consume();
			return 0;
		}else if (con_pids[i] < 0){
			printf("Error encountered while creating the consumer process\n");
			return -1;
		}
	}
	/*for (i=0; i<N; i++) printf("Consumer process #%d PID: %d\n", i, con_pids[i]);*/
	sem_unlink(EMPTY_SEM_NAME);
	sem_unlink(FULL_SEM_NAME);
	sem_unlink(MUTEX_SEM_NAME);
	wait(NULL);
	return 0;
}

void produce(){
	int i;
	for (i=0; i<=M; i++){
		sem_wait(empty_sem);
		sem_wait(mutex_sem);
		lseek(fd, (in+1)*sizeof(int), SEEK_SET);
		write(fd, (char*)(&i), sizeof(int));
		in = (in+1) % 10;
		sem_post(mutex_sem);
		sem_post(full_sem);
	}
}

void consume(){
	int i;
	int out_pos, result;
	for (i=0; i<M/N; i++){
		sem_wait(full_sem);	
		sem_wait(mutex_sem);
		lseek(fd, 0, SEEK_SET);
		read(fd, (char*)(&out_pos), sizeof(int));
		lseek(fd, (out_pos+1)*sizeof(int), SEEK_SET);
		read(fd, (char*)(&result), sizeof(int)); 
		out_pos = (out_pos+1) % 10;
		lseek(fd, 0, SEEK_SET);
		write(fd, (char*)(&out_pos), sizeof(int));
		printf("%d: %d\n", getpid(), result);
		fflush(stdout);
		sem_post(mutex_sem);
		sem_post(empty_sem);
	}
}