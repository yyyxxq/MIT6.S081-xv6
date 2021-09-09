#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
char* fmtname(char *path){
	static char buf[DIRSIZ+1];
	char *p;
	// Find first character after last slash.
	for(p=path+strlen(path); p >= path && *p != '/'; p--);
	p++;
	// Return blank-padded name.
	if(strlen(p) >= DIRSIZ)
	return p;
	memmove(buf, p, strlen(p));
	memset(buf+strlen(p), 0, DIRSIZ-strlen(p));
	return buf;
}
void find(char *path,char *name){
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(path, 0)) < 0){
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}

	strcpy(buf, path);
	p = buf+strlen(buf);
	*p++ = '/';
	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if(de.inum == 0)
		continue;
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		if(stat(buf, &st) < 0){
			printf("ls: cannot stat %s\n", buf);
			continue;
		}
		if(st.type==2){
			if(strcmp(fmtname(buf),name)==0)printf("%s\n",buf);
		}
		if(st.type==1){
			if(strcmp(fmtname(buf),".")!=0&&strcmp(fmtname(buf),"..")!=0)find(buf,name);
		}
	//  printf("%s %d %d %d\n", buf, st.type, st.ino, st.size);
	}
	close(fd);
}
int main(int argc, char *argv[]){
	find(argv[1],argv[2]);
	exit(0);
}