#include <sys/syscall.h>
#include <unistd.h>

int main(){
	int prio=0;
//	syscall(332,4644,0,0,&prio);
	syscall(332,4644,1,-5,&prio);
	printf("%d\n",prio);
	return 0;;
}
