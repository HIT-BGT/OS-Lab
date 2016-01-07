#include <fcntl.h>	// To use O_* procedures
#include <sys/types.h> // To use "pid_t" type
#include <sys/stat.h> // To use permission(S_*) values
#include <semaphore.h>
#include <unistd.h>	// To use fork()
#include <stdio.h>	// To use printf()
#include <stdlib.h>	// To use freopen()
#include <wait.h>	// To use wait()

#define EMPTY_SEM_NAME "/empty_sem_name"
#define EMPTY_SEM_VALUE 10
#define FULL_SEM_NAME "/full_sem_name"
#define FULL_SEM_VALUE 0
#define MUTEX_SEM_NAME "/mutex_sem_name"
#define MUTEX_SEM_VALUE 1
#define N 5	//The number of consumer processes
#define M 501

// Declaration of producer process
void produce();

// Declaration of consumer process
void consume();

sem_t *empty_sem;
sem_t *full_sem;
sem_t *mutex_sem;
int fd; // The file description
int out_fd;	// Print result to this file
int in = 0;
int out = 0; 

int main(int argc, char const *argv[])
{
	int i;
	freopen("data.out","w",stdout);  // Redirect output to a file
	empty_sem = sem_open(EMPTY_SEM_NAME, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, EMPTY_SEM_VALUE); // mode = 00700|00070|00007
	full_sem = sem_open(FULL_SEM_NAME, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, FULL_SEM_VALUE); // mode = 00700|00070|00007
	mutex_sem = sem_open(MUTEX_SEM_NAME, O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, MUTEX_SEM_VALUE); // mode = 00700|00070|00007
	fd = open("buffer", O_CREAT|O_TRUNC|O_RDWR, 0666);
	if (fd<0){
		printf("Failed to open buffer file!\n");
		return -1;
	}
	lseek(fd, 0, SEEK_SET);	// Set RW position to start of file
	write(fd, (char*)(&out), sizeof(int));	// Write read position to the buffer file
	// Create a producer process
	pid_t pro_pid = fork();
	if (pro_pid==0){
		produce();
		return 0;	// Finish the producer process
	}else if(pro_pid<0){
		// Error encountered when create a producer process
		printf("Error encountered while creating a producer process!\n");
		return -1;
	}else{
		// This is in the parent process
		// printf("Producer process PID: %d\n", pro_pid);
	}
	pid_t con_pids[N];
	for (i=0; i<N; i++){
		// Create a consumer process
		con_pids[i] = fork();
		if (con_pids[i] == 0){
			// In the consumer process
			consume();
			return 0;	// Finish the consumer process
		}else if (con_pids[i] < 0){
			printf("Error encountered while creating the consumer process\n");
			return -1;
		}
	}
	// for (i=0; i<N; i++) printf("Consumer process #%d PID: %d\n", i, con_pids[i]);
	sem_unlink(EMPTY_SEM_NAME);
	sem_unlink(FULL_SEM_NAME);
	sem_unlink(MUTEX_SEM_NAME);
	for (i=0; i<N+1; i++) wait(NULL);
	return 0;
}

// Definition of producer process
void produce(){
	int i;
	for (i=0; i<=M; i++){
		sem_wait(empty_sem);	// P(Empty)
		sem_wait(mutex_sem);	// P(Mutex)
		lseek(fd, (in+1)*sizeof(int), SEEK_SET); // Move RW position to end of file
		write(fd, (char*)(&i), sizeof(int));	// Put item into buffer
		in = (in+1) % 10;
		sem_post(mutex_sem);	// V(Mutex)
		sem_post(full_sem);		// V(Full)
	}
}

// Definition of consumer process
void consume(){
	int i;
	int out_pos, result;
	for (i=0; i<M/N; i++){
		sem_wait(full_sem);		// P(Full)
		sem_wait(mutex_sem);	// P(Mutex)
		lseek(fd, 0, SEEK_SET);	// Move RW position to start of file
		read(fd, (char*)(&out_pos), sizeof(int));	// Read the read posotion
		lseek(fd, (out_pos+1)*sizeof(int), SEEK_SET); // Move RW position to read position
		read(fd, (char*)(&result), sizeof(int)); 	// Read the result
		out_pos = (out_pos+1) % 10;
		lseek(fd, 0, SEEK_SET);	// Move RW position to start of file
		write(fd, (char*)(&out_pos), sizeof(int));	// Write the new read posotion
		printf("%d: %d\n", getpid(), result);
		sem_post(mutex_sem);	// V(Mutex)
		sem_post(empty_sem);	// V(Empty)
	}
}