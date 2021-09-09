#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(int argc,char *argv[]){
//	printf("%d\n",argc);
//	for(int i=0;i<argc;i++)printf("%s\n",argv[i]);
	if(argc!=2){
		printf("少了个描述时间的数字\n");
	}else{
		sleep(atoi(argv[1]));
	}
	exit(0);
}