#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>

#define MAX_READ 512
#define MAX_INT 5


void input(int fd, char *path);
void find(int fd, char *name);
void export(int fd, char *name, char *dest);
void delete(int fd, char *path, char *name);
int exists (int fd, char *name);  //if found returns start of object, if not found returns -1
int find_end(int fd, char *name, int start);
void copy(int target, int source, int start, int end);

int main(int argc,char *argv[]){

    int fd;
	char action[MAX_READ]/*, *dest, name[MAX_READ]*/;

    if((fd = open(argv[1],O_RDWR|O_CREAT,0666))==-1){
        printf("open %d\n",errno);
    }

	//check() for valid db
    
	//menu
    printf("Select action (i,f,e,d,q)");
	fgets(action,MAX_READ,stdin);
    while(action[0]!='q'){
		action[strcspn(action,"\n")]='\0';  //remove \n from end of fgets() input
		if(action[1]!=' '){
			printf("Invalid input\n");
		}
		else{
			switch(action[0]){
				case 'i':
					input(fd,&action[2]);
					break;
				case 'f':
				//	find(fd,&action[2]);
					break;
				case 'e':
				/*	if(strchr(&action[2],' ')==NULL){
						printf("Invalid input\n");
						break;
					}
					dest=strchr(&action[2],' ')+sizeof(char); //dest from empty+1 to end
					strncpy(name,&action[2],strlen(&action[2])-strlen(dest));
					name[strlen(&action[2])-strlen(dest)-1]='\0';
					export(fd, name, dest);*/
					break;
				case 'd':
					//delete(fd, argv[1], &action[2]);
					break;
				case 'q':
					close(fd);
					break;
				default: 
					printf("Invalid input\n");
				}
		}
		//	break;
        printf("Select action (i,f,e,d,q)");
		fgets(action,MAX_READ,stdin);
    }

	if(close(fd)==-1){
		printf("%d error\n",errno);
	}

    return 0;
}


void input(int fd, char *path){

	int fd2,bytes,size=0;
	char buf[MAX_READ];
	char int_str[MAX_READ], name_size_str[MAX_READ];
	char *name;

	if ((fd2=open(path,O_RDONLY))==-1){
		printf("Error %d when opening fd2\n",errno);
		return;
	}

	char format_size[20]; //format_size of data size
	sprintf(format_size,"<%%0%dd>",MAX_INT);


	name=strrchr(path,'/'); //from path to name
	if(name==NULL){
		name=path;
	}

	//check if name already exists in db
	if(exists(fd,name)!=-1){
		printf("Error, item already exists in db\n");
		return;
	}
	
	lseek(fd,(off_t) 0,SEEK_END); //put cursor on EOF if db already exists

	//input name size
	sprintf(name_size_str,format_size,strlen(name));
	write(fd,name_size_str,strlen(name_size_str));
	//input name
	write(fd,name,strlen(name));

	//place size placeholder
	sprintf(int_str,format_size,0);
	write(fd,int_str,strlen(int_str));

	//Write data
	while((bytes=read(fd2,buf,MAX_READ))>0){
		write(fd,buf,bytes);
		size+=bytes;
	}

	//overwrite placeholder to correct size of data
	lseek(fd,(off_t)-(size+MAX_INT+2),SEEK_CUR); //find placeholder before data (+2 for brackets)
	sprintf(int_str,format_size,size);
	write(fd,int_str,strlen(int_str));
	
	close(fd2);
}

/*void find(int fd,char *name){
	char buf[MAX_READ],haystack[MAX_READ],*ptr;

	lseek(fd,(off_t) 1,SEEK_SET);
	while(read(fd,buf,MAX_INT)!=0){
		buf[MAX_INT]='\0';
		lseek(fd,(off_t)1,SEEK_CUR);
		read(fd,haystack,strtol(buf,&ptr,10));
		haystack[strtol(buf,&ptr,10)]='\0';
		if(strstr(haystack,name)||(strcmp(name,"*"))==0){
			printf("%s\n",haystack);
		}
		lseek(fd,(off_t)1,SEEK_CUR);		
		read(fd,buf,MAX_INT); //skip data
		buf[MAX_INT]='\0';
		lseek(fd,(off_t) strtol(buf,&ptr,10)+2,SEEK_CUR);
	}
}

void export(int fd, char *name, char *dest){

	int pos, fd2, end;
	
	if ((pos=exists(fd,name))==-1){
		printf("Object not found in Database\n");
		return;
	}
	if((fd2 = open(dest,O_RDWR|O_CREAT|O_EXCL,0666))==-1){
        printf("Export:File already exists %d\n",errno);
		return;
    }

	end=find_end(fd,name,pos);	
	//write to dest
	copy(fd2,fd,pos,end);
}

void delete(int fd, char *path, char *name){

	int fd2,pos;

	if((fd2 = open(path,O_RDONLY,0666))==-1){
        printf("open %d\n",errno);
    }

	if((pos=exists(fd,name))==-1){
		printf("Object not found in Database\n");
		return;
	}

	int start =find_end(fd,name,pos);
	int end=lseek(fd2,(off_t)0,SEEK_END);
	lseek(fd,(off_t)pos,SEEK_SET);

	copy(fd,fd2,start,end);

	ftruncate(fd,end-start+pos);

	if(close(fd2)==-1){
		printf("%d error\n",errno);
	}
}*/

int exists (int fd, char *name){

	char buf[MAX_READ];
	char *ptr;

	lseek(fd,(off_t) 1,SEEK_SET);
	while(read(fd,buf,MAX_INT)!=0){
		buf[MAX_INT]='\0';
		if(strlen(name)==strtol(buf,&ptr,10)){  //if same size
			lseek(fd,(off_t)1,SEEK_CUR);
			read(fd,buf,strlen(name)); 
			buf[strlen(name)]='\0';
			if(strcmp(name,buf)==0){  //if same name
				return lseek(fd,(off_t)-strlen(name)-MAX_INT-2,SEEK_CUR);;
			}
			else{
				lseek(fd,(off_t)1,SEEK_CUR);
			}
		}
		else{
			lseek(fd,(off_t) strtol(buf,&ptr,10)+2,SEEK_CUR);
		}
		read(fd,buf,MAX_INT); //skip data
		buf[MAX_INT]='\0';
		lseek(fd,(off_t) strtol(buf,&ptr,10)+2,SEEK_CUR);
	}
	return -1;
}

/*int find_end(int fd, char *name, int start){
	char buf[MAX_READ], *ptr;
	
	lseek(fd,(off_t) start+3+strlen(name)+MAX_INT, SEEK_SET);  //find end
	read(fd,buf,MAX_INT); //skip data
	buf[MAX_INT]='\0';
	return lseek(fd,(off_t) strtol(buf,&ptr,10)+1,SEEK_CUR);
}

void copy(int target, int source, int start, int end){
	char buf[MAX_READ];

	lseek(source,(off_t) start, SEEK_SET);
	for(int i=0;i<(end-start)/MAX_READ;i++){  //get object
		read(source,buf,MAX_READ);
		write(target,buf,MAX_READ);
	}
	read(source,buf,(end-start)%MAX_READ);
	write(target,buf,(end-start)%MAX_READ);
}*/