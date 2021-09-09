#include "kernel/types.h"
#include "user/user.h"
int flag=0,prime=0;
int from[2],to[2],p[2];
int receive(int x);
int transport(int x);
int receive(int x){
	if(x==-1)exit(0);
	printf("prime %d\n",x);
	flag=1;
	prime=x;
	pipe(p);
	int pid=fork();
	if(pid==0){
		flag=0;
		from[0]=p[0];
		from[1]=p[1];
		close(from[1]);
		while(x!=-1){
			read(from[0],&x,4);
			transport(x);
		}
		close(from[0]);
		wait(0);
		exit(0);
	}else{
		to[0]=p[0];
		to[1]=p[1];
		close(to[0]);
	}
	return 0;
}
int transport(int x){
	if(flag==0)receive(x);
	else{
		if(x%prime!=0||prime==0)write(to[1],&x,4);
	}
	return 0;
}
int main(){
	for(int i=2;i<=35;i++)transport(i);
	transport(-1);
	close(to[1]);
	wait(0);
	exit(0);
}