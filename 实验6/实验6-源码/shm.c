#define __LIBRARY__
#include <unistd.h>  
#include <linux/kernel.h>
#include <errno.h>	/* To use ENOMEM and EINVAL*/
#include <linux/sched.h>  
#include <asm/system.h>  
#include <linux/mm.h>

struct shm_node
{
	int occupied;	// Indicate whether this node has been occupied
	int phy_addr;
} shm_table[20];

int sys_shmget(int key, size_t size, int shmflg){
	int phy_addr;	// Store the physical address returned by get_free_page() tempporily
	if(shm_table[(int)key].occupied == 1)	// If already exists
		return (int)key;	//	Return it
	if ((int)size > 4096) return -(EINVAL);	// If size is bigger than a page
	if (0==(phy_addr=get_free_page())) return -(ENOMEM);	// If no free memory
	shm_table[(int)key].occupied=1;	// Set to be occupied
	shm_table[(int)key].phy_addr=phy_addr;	// Remember the physical addr
	return (int)key;	// Return the physical addr
}
void* sys_shmat(int shmid, const void *shmaddr, int shmflg){
	if(shm_table[shmid].occupied != 1)
	{
		printk("This shmid is invalid!\n");
		return -(EINVAL);
	}
	put_page(shm_table[shmid].phy_addr, current->start_code + current->brk);
	return (void*)current->brk;
}