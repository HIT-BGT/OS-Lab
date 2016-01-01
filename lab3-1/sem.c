#include <unistd.h>
#include <string.h>	// To use strcmp() and strcpy()
#include <linux/kernel.h>	// To use malloc()
#include <asm/segment.h>	// To use get_fs_byte()
#include <asm/system.h>	// To use get_fs_byte()
#include <linux/sched.h>	// To use current, sleep_on() and wake_up()

sem_node* sem_queue_head = NULL;

sem_t* sys_sem_open(const char* name, unsigned int value){
	char tmp[24];	// Store the name temporily
	int count;
	for (count = 0; count < 23 ; count++){
         if (get_fs_byte(name+count) == '\0') break;
         tmp[count] = get_fs_byte(name+count);
      }
     tmp[count] = '\0';
     if (get_fs_byte(name+count) != '\0'){
     	printk("Name too long!");
     	return NULL;     	
     }
    sem_node* pre_sem_ptr = NULL;	// Previous sem node checked
	sem_node* cur_sem_ptr = NULL;	// Current sem node being ckecked
	for (cur_sem_ptr=sem_queue_head; cur_sem_ptr!=NULL; cur_sem_ptr=cur_sem_ptr->next){
		if (strcmp(cur_sem_ptr->sem_ptr->name, tmp) == 0)
			return cur_sem_ptr->sem_ptr;	// If found, just open it
		pre_sem_ptr = cur_sem_ptr;	// Record the last sem node
	}
	/* Apply for a new semphore queue node and set its values */
	cur_sem_ptr = (sem_node*)malloc(sizeof(sem_node));	// Create a new sem node
	cur_sem_ptr->next = NULL;	// Set new node's next
	cur_sem_ptr->sem_ptr = (sem_t*)malloc(sizeof(sem_t));	// Create a new semaphore
	strcpy(cur_sem_ptr->sem_ptr->name, tmp);	// Set semaphore's name
	cur_sem_ptr->sem_ptr->val = (int*)malloc(sizeof(int));
	*(cur_sem_ptr->sem_ptr->val) = value; // Set semaphore's value
	cur_sem_ptr->sem_ptr->wait_queue_head = NULL;	// Set semaphore's wait queue
	/*********************************************************/

	if (sem_queue_head == NULL)  // If this is the first semaphore
		sem_queue_head = cur_sem_ptr;
	else	// Else append it to the semaphore queue
		pre_sem_ptr->next = cur_sem_ptr;
	// printk("Open semaphore: %s\n", tmp);
	printk("Current semaphore queue:");
	for (pre_sem_ptr=sem_queue_head; pre_sem_ptr!=NULL; pre_sem_ptr=pre_sem_ptr->next)
		printk("%s%d ", pre_sem_ptr->sem_ptr->name, *(pre_sem_ptr->sem_ptr->val));
	printk("\n");
	return cur_sem_ptr->sem_ptr;
}

int sys_sem_wait(sem_t* sem){
	// printk("sem_wait: %s, %d\n", sem->name, *(sem->val));
	cli();	// Atom operation
	*(sem->val) = *(sem->val)-1;
	if (*(sem->val)<0){	// Make the current process sleep
		task_node* pre_task_ptr = NULL; // Point to previous node
		task_node* cur_task_ptr = NULL;	// Point to current task node
		for (cur_task_ptr=sem->wait_queue_head; cur_task_ptr!=NULL; cur_task_ptr=cur_task_ptr->next)
			pre_task_ptr = cur_task_ptr;
		cur_task_ptr = (task_node*)malloc(sizeof(task_node));	// Create a new task node
		cur_task_ptr->task_ptr = current;	// Set the task ptr
		cur_task_ptr->next = NULL;	// Set new task node's next
		if (sem->wait_queue_head==NULL) // No process waiting
			sem->wait_queue_head = cur_task_ptr;
		else
			pre_task_ptr->next = cur_task_ptr;
		sleep_on(&current);	// Make current process sleep
	}
	sti();
	return 0;
}

int sys_sem_post(sem_t* sem){
	// printk("sem_post: %s, %d\n", sem->name, *(sem->val));
	cli();	// Atom operation
	if (sem->wait_queue_head!=NULL){	// One or more tasks are hanging
		struct task_struct* task_ptr = sem->wait_queue_head->task_ptr;	// Get the first task pointer
		sem->wait_queue_head = sem->wait_queue_head->next;	// Delete the first waiting task queue node
		wake_up(&task_ptr);	// Wake up the first node
	}
	else
		*(sem->val) = *(sem->val)+1;
	sti();
	return 0;
}

int sys_sem_unlink(const char* name){
	char tmp[24];	// Store the name temporily
	int count;
	for (count = 0; count < 23 ; count++){
         if (get_fs_byte(name+count) == '\0') break;
         tmp[count] = get_fs_byte(name+count);
      }
     tmp[count] = '\0';
     if (get_fs_byte(name+count) != '\0'){
     	printk("Name too long!");
     	return -1;
     }
     int found = 0;
	 sem_node* pre_sem_ptr = NULL; // Previous sem node checked
     sem_node* cur_sem_ptr = NULL;	// Current sem node being ckecked
	for (cur_sem_ptr=sem_queue_head; cur_sem_ptr!=NULL; cur_sem_ptr=cur_sem_ptr->next){
		if (strcmp(cur_sem_ptr->sem_ptr->name, tmp) == 0){
			found = 1;	// Set found flag
			break;
		}
		pre_sem_ptr = cur_sem_ptr;	// Record the last sem node
	}
	if (found==1){
		if (pre_sem_ptr==NULL)	// Found the first node
			sem_queue_head = sem_queue_head->next;
		else
			pre_sem_ptr->next = cur_sem_ptr->next;
		return 0;
	}
	return -1;
}
