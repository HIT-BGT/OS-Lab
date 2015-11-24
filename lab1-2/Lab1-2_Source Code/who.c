#define __LIBRARY__              /* 有它，_syscall1等才有效。详见unistd.h */
#include <unistd.h>           /* 有它，编译器才能获知自定义的系统调用的编号 */
#include <errno.h>            //in order to use errno
#include <string.h>            //in order to use "strcpy"
#include <asm/segment.h>      //in order to use get_fs_byte and put_fs_byte
   
char str[24];
int sys_iam(const char * name)
{
      //char* ptr;   //cannot do it this way, cause ptr points to a address in kernel state
      int count;
      char tmp[24];  //no changing the str when the name is too long
      for (count = 0; count < 23 ; count++){
         if (get_fs_byte(name+count) == '\0') break;
         tmp[count] = get_fs_byte(name+count);
      }
      tmp[count] = '\0';
      //printk("Holy Shit");  //just ensure system call has succeeded
      if (get_fs_byte(name+count) == '\0'){
         strcpy(str, tmp);
         return count;
      }
      return -(EINVAL);
}
int sys_whoami(char* name, unsigned int size)
{
   int count;
   for (count=0; count<size; count++){
      if (str[count] == '\0') break;
      put_fs_byte(str[count], name+count);
   }
   put_fs_byte('\0', name+count);
   if (str[count] == '\0') return count;
   return -(EINVAL);
}