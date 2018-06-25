#include <unistd.h>
#include <string.h>
#include <errno.h>
#include<stdio.h>
#include <fcntl.h>
#include "db.h"
#include "util.h"

void input(int fd, char *path){

	int fd2,bytes,temp,size=0;
	char buf[MAX_READ];
	char *name;

	if ((fd2=open(path,O_RDONLY))==-1){
		printf("Error %d when opening fd2\n",errno);
		return;
	}

	if(strrchr(path,'/')==NULL){
		name=path;
	}
	else{
		name=strrchr(path,'/')+sizeof(char);
	}

	//check if name already exists in db
	if(exists(fd,name)!=-1){
		printf("Error, item already exists in db\n");
		return;
	}
	
	lseek(fd,(off_t) 0,SEEK_END); //put cursor on EOF
	temp=strlen(name); //input name size
	write(fd,&temp,sizeof(int));
	write(fd,name,strlen(name));  //input name
	write(fd,&temp,sizeof(int));

	//Write data
	while((bytes=read(fd2,buf,MAX_READ))>0){
		write(fd,buf,bytes);
		size+=bytes;
	}

	lseek(fd,(off_t)-(size+sizeof(int)),SEEK_CUR); //find placeholder before data
	write(fd,&size,sizeof(int));  //overwrite placeholder to correct size of data
	
	if(close(fd2)==-1){
		printf("%d error\n",errno);
	}}

void find(int fd,char *name){
	char buf[MAX_READ];
	int temp;

	lseek(fd,(off_t) 0,SEEK_SET);
	while(read(fd,&temp,sizeof(int))!=0){
		read(fd,buf,temp);
		buf[temp]='\0';
		if(strstr(buf,name)||(strcmp(name,"*"))==0){
			printf("%s\n",buf);
		}
		read(fd,&temp,sizeof(int)); //skip data
		lseek(fd,(off_t) temp,SEEK_CUR);
	}
}

void export(int fd, char *command){
	int pos, fd2, end;
	char name[MAX_READ], *dest;

	dest=strchr(command,' ')+sizeof(char); //dest from empty+1 to end
	strncpy(name,command,strlen(command)-strlen(dest));
	name[strlen(command)-strlen(dest)-1]='\0';
	
	if ((pos=exists(fd,name))==-1){
		printf("Object not found in Database\n");
		return;
	}
	if((fd2 = open(dest,O_RDWR|O_CREAT|O_EXCL,0777))==-1){
        printf("Export:File already exists %d\n",errno);
		return;
    }

	end=find_end(fd,name,pos);	
	copy(fd2,fd,lseek(fd,(off_t)pos+2*sizeof(int)+strlen(name),SEEK_SET),end); //write to dest
	
	if(close(fd2)==-1){
		printf("%d error\n",errno);
	}
}

void delete(int fd, char *path, char *name){

	int fd2, pos;
	int start, end;

	if((fd2 = open(path,O_RDONLY,0666))==-1){
        printf("open %d\n",errno);
    }

	if((pos=exists(fd,name))==-1){
		printf("Object not found in Database\n");
		return;
	}

	start =find_end(fd,name,pos);
	end=lseek(fd2,(off_t)0,SEEK_END);
	lseek(fd,(off_t)pos,SEEK_SET);
	copy(fd,fd2,start,end);  //transfer data over those to delete
	ftruncate(fd,end-start+pos);  //reduce file size

	if(close(fd2)==-1){
		printf("%d error\n",errno);
	}
}