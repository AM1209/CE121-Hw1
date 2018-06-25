#include<stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "db.h"

int main(int argc,char *argv[]){

    int fd;
	char action[MAX_READ];

	if (argv[1]==NULL){
		printf("Error Invalid input\n");
		return 0;
	}

    if((fd = open(argv[1],O_RDWR|O_CREAT,0666))==-1){
        printf("open %d\n",errno);
    }

	//check() for valid db
    
    printf("Select action (i,f,e,d,q)");

    while(fgets(action,MAX_READ,stdin)[0]!='q'){

		action[strcspn(action,"\n")]='\0';
		
		if(action[1]!=' '){
			printf("Invalid input\n");
			printf("Select action (i,f,e,d,q)");
			continue;
		}
		
		switch(action[0]){
			case 'i':
				input(fd,&action[2]);
				break;
			case 'f':
				find(fd,&action[2]);
				break;
			case 'e':
				if(strchr(&action[2],' ')==NULL){
					printf("Invalid input\n");
					break;
				}
				export (fd,&action[2]);
				break;
			case 'd':
				delete(fd, argv[1], &action[2]);
				break;
			case 'q':
				close(fd);
				break;
			default: 
				printf("Invalid input\n");
		}
		
        printf("Select action (i,f,e,d,q)");
    }

	if(close(fd)==-1){
		printf("%d error\n",errno);
	}

    return 0;
}