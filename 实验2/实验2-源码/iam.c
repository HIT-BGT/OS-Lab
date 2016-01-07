#define __LIBRARY__
#include <unistd.h>

_syscall1(int, iam, const char*, name);

int main(int agc, char** argv){
	//printf("%d\n", iam(argv[1]));
	iam(argv[1]);
	return 0;
}