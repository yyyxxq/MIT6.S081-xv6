#include "kernel/types.h"
#include "user/user.h"
int main(){
	int pid;
	int p[2];
	pipe(p);
	pid=fork();
	if(pid==0){
		printf("%d: received ping\n",getpid());
		close(p[0]);
		write(p[1],"ping-pong",9);
	}else{
		close(p[1]);
		char a[10];
		read(p[0],a,9);
	//	printf("%s\n",a);
		printf("%d: received pong\n",getpid());
	}
	exit(0);
}