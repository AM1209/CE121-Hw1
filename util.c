#include <unistd.h>
#include <string.h>
#include "util.h"


int exists (int fd, char *name){

	char buf[MAX_READ];
	int temp;

	lseek(fd,(off_t)0,SEEK_SET);
	while(read(fd,&temp,sizeof(int))!=0){
		if(strlen(name)==temp){  //if same size
			read(fd,buf,strlen(name)); 
			buf[strlen(name)]='\0';
			if(strcmp(name,buf)==0){  //if same name
				return lseek(fd,(off_t)-strlen(name)-sizeof(int),SEEK_CUR);;
			}
		}
		else{
			lseek(fd,(off_t) temp,SEEK_CUR);
		}
		read(fd,&temp,sizeof(int)); //skip data
		lseek(fd,(off_t) temp,SEEK_CUR);
	}
	return -1;
}

int find_end(int fd, char *name, int start){
	int temp;

	lseek(fd,(off_t) start+sizeof(int)+strlen(name), SEEK_SET);  //find end
	read(fd,&temp,sizeof(int)); //skip data
	return lseek(fd,(off_t) temp,SEEK_CUR);
}

void copy(int target, int source, int start, int end){
	char buf[MAX_READ];

	lseek(source,(off_t) start, SEEK_SET);
	for(int i=0;i<(end-start)/MAX_READ;i++){
		read(source,buf,MAX_READ);
		write(target,buf,MAX_READ);
	}
	read(source,buf,(end-start)%MAX_READ);
	write(target,buf,(end-start)%MAX_READ);
}