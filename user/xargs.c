#include "kernel/types.h"
#include "user/user.h"
char a[120][120];
char *b[120];
int c[120];
int row,column;
char tmp;
int solve(char s[],int argc,char *argv[]){
	int n=strlen(s);
	int A=0;
	for(int i=1;i<argc;i++){
		b[A]=argv[i];
		A++;
	}
	b[A]=&s[0];
	A++;
	for(int i=0;i<n;i++){
		if(s[i]==' '){
			s[i]=0;
			b[A]=&s[i+1];
			A++;
		}
	}
	b[A]=0;
	//for(int i=0;i<A;i++)printf("AA::%s\n",b[i]);
	exec(argv[1],b);
	exit(0);
}
int main(int argc,char *argv[]){
	while(read(0,&tmp,1)){
		if(tmp=='\n'){
			row++;
			column=0;
		//	a[row]=new char[120];
			continue;
		}
		a[row][column++]=tmp;
	}
	for(int i=0;i<row;i++){
		int pid=fork();
		if(pid==0){
			solve(a[i],argc,argv);
			exit(0);
		}
	}
//	for(int i=0;i<row;i++)wait(0);
	while(wait(0)!=-1);
	exit(0);
}