/*Giorgos Dimos
 *06/05/2020
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_ARGS 3
#define MAX_READ 512
#define MAX_STR 255
#define FILENAMELEN 1
#define MAXFILESIZE 8

#define CHECK(sinartisi)\
    if((sinartisi) == -1){\
        fprintf(stderr,"errno: %d at line: %d\n", errno, __LINE__);\
        return -1;\
    }\

static int mystrtok(const char *str, char delim, char *tokenizedStr[], int arraySize);
static int search(int fd, const char *fileToFind);
int import(int fd, const char *importFile);
int find(int fd, const char *fileToFind);
int export(int fd, const char *src, const char *dst);
int delete(int fd, const char *name, const char *file);

int main(int argc, char *argv[]){

    int fd,numOfArg;
    char user_input[MAX_STR-1], *tokenized_input[MAX_ARGS];

    if(argc!=2){
        printf("Invalid number of arguments\n");
        printf("Please enter the name of the database file\n");
        return 1;
    }

    CHECK(fd=open(argv[1], O_RDWR|O_CREAT, S_IRWXU));

    while(fgets(user_input, MAX_STR-1, stdin)){
        
        user_input[strcspn(user_input,"\n")]='\0';
        numOfArg=mystrtok(user_input, ' ', tokenized_input, MAX_ARGS);
        if(numOfArg<=0){
            printf("Invalid number of arguments! Try again!\n");
            continue;
        }

        if(numOfArg==2 && !strcmp("i",tokenized_input[0])){
            if(!strcmp(argv[1], tokenized_input[1])){
                printf("Cant import the database file to the database\n");
                continue;
            }
            printf("Import %s\n", tokenized_input[1]);
            if(import(fd,tokenized_input[1]))
                printf("Import failed\n");
        }
        else if(numOfArg==2 && !strcmp("f",tokenized_input[0])){
            printf("Find %s\n", tokenized_input[1]);
            if(!find(fd,tokenized_input[1]))
                printf("File not found\n");
        }
        else if(numOfArg==3 && !strcmp("e",tokenized_input[0])){
            printf("Export %s to %s\n", tokenized_input[1], tokenized_input[2]);
            if(export(fd,tokenized_input[1], tokenized_input[2]))
                printf("Export failed\n");
        }
        else if(numOfArg==2 && !strcmp("d",tokenized_input[0])){
            printf("Deleting %s\n", tokenized_input[1]);
            if(delete(fd,tokenized_input[1], argv[1]))
                printf("Delete failed\n");
        }
        else if(numOfArg==1 && !strcmp("q",tokenized_input[0])){
            printf("quit\n");
            break;
        }
        else {
            printf("Invalid input\n");
        }

        for(int i=0; i<numOfArg; i++){
            free(tokenized_input[i]);
        }
    }

    for(int i=0; i<numOfArg; i++){
        free(tokenized_input[i]);
    }

    CHECK(close(fd))

    return 1;
}

static int mystrtok(const char *str, char delim, char *tokenizedStr[], int arraySize){
    int i=0;
    size_t len;

    if(!*str || arraySize<=0){
        return 0;
    }

    while(1){
        
        if(*str=='\0')  //reached end of string return number of tokens
            return i;
    
        if(i == arraySize){  //reached Array limits, string not fully tokenized, return failure
            for (int j=0;j< arraySize; j++){
                free(tokenizedStr[j]);
            }
            return -1;
        }

        while(*str==delim) //discard delim characters
            str++; 

        len=0;  
        while(str[len]!=delim && str[len]!='\0') //find length of substring
            len++;

        tokenizedStr[i] = (char *)malloc(len+1);  //malloc memory for new string
        if(!tokenizedStr[i]){
            for (int j=0;j< i; j++){
                free(tokenizedStr[j]);
            }
            fprintf(stderr,"Malloc failed\n");
            return -1;
        }
        strncpy(tokenizedStr[i], str, len);
        tokenizedStr[i][len]='\0';

        str+=len;
        i++;
    }
}

static int search(int fd, const char *fileToFind){
    unsigned char buf[MAX_READ];
    ssize_t readBytes=0, fileSize=0, offset=0;

    CHECK(lseek(fd, 0, SEEK_SET));

    while((readBytes=read(fd, buf, FILENAMELEN))){  //diabazei mikos onomatos

        CHECK(readBytes);//elegxei an epestrepse lathos

        CHECK(readBytes = read(fd, buf, (size_t)*buf));  //Diabazei to onoma
        buf[readBytes]='\0';

        if(!strcmp((char *)buf, fileToFind)){  //elegxei an exoun paromoio onoma
            CHECK(offset = lseek(fd, -(strlen(fileToFind)+FILENAMELEN), SEEK_CUR));  //Telos arxeiou
            return offset;
        }

        CHECK(read(fd, &fileSize, MAXFILESIZE));  //Diabazei to megethos tou arxeiou

        CHECK(lseek(fd, fileSize, SEEK_CUR));  //Telos arxeiou
    }
    return -1;
}

int import(int fd, const char *importFile){
    int fdInputFile;
    const char *importFileName;
    ssize_t readBytes, totalBytes;
    size_t strlenToWrite;
    unsigned char buf[MAX_READ];

    CHECK(fdInputFile=open(importFile, O_RDONLY));

    if(!(importFileName = strrchr(importFile,'/'))){
        importFileName = importFile;
    }
    else{
        importFileName++;
    }

    if(search(fd,importFileName)>=0){
        printf("File already exists in DB\n");
        return 1;
    }

    CHECK(lseek(fd, 0, SEEK_END));  //Write to end of the file

    strlenToWrite = strlen(importFileName);  //First write the length of the file's name
    CHECK(write(fd, &strlenToWrite, FILENAMELEN));
  
    CHECK(write(fd, importFileName, strlen(importFileName)));  //then write the name of the file

    CHECK(write(fd, "", MAXFILESIZE));  //put a placeholder for the size of the file
                                        //same thing could be done with stat command

    totalBytes = 0;
    while((readBytes = read(fdInputFile, buf, MAX_READ))){  //write all bytes of the file to the bd
        CHECK(readBytes);
        CHECK(write(fd, buf, readBytes));
        totalBytes+=readBytes;
    }

    CHECK(lseek(fd, -(totalBytes+MAXFILESIZE), SEEK_CUR));  //go back to placeholder

    CHECK(write(fd, &totalBytes, MAXFILESIZE));  //and write the actual file size

    CHECK(close(fdInputFile));

    return 0;
}

int find(int fd, const char *fileToFind){
    unsigned char buf[MAX_READ];
    ssize_t readBytes=0, fileSize=0;
    int found = 0;

    CHECK(lseek(fd, 0, SEEK_SET));

    while((readBytes=read(fd, buf, FILENAMELEN))){  //diabazei mikos onomatos

        CHECK(readBytes);  //elegxei an epestrepse lathos

        CHECK(readBytes = read(fd, buf, (size_t)*buf));  //Diabazei to onoma

        buf[readBytes]='\0';

        if(strstr((char *)buf,fileToFind) || !strcmp("*",fileToFind)){  //elegxei an exoun paromoio onoma
            printf("%s\n",buf);
            found++;
        }

        CHECK(read(fd, &fileSize, MAXFILESIZE));  //Diabazei to megethos tou arxeiou

        CHECK(lseek(fd, fileSize, SEEK_CUR));  //Telos arxeiou
  
    }
    return found;
}

int export(int fd, const char *src, const char *dest){

    int fd_dst, offset, readBytes, fileSize;
    unsigned char buf[MAX_READ];

    if((offset = search(fd,src))==-1){
        return -1;
    }

    CHECK(fd_dst=open(dest, O_RDWR|O_CREAT|O_EXCL, S_IRWXU));

    CHECK(lseek(fd, offset+FILENAMELEN+strlen(src), SEEK_SET));  //Arxh data

    CHECK(read(fd, &fileSize, MAXFILESIZE)); //Diabazei to megethos tou arxeiou

    while(fileSize){
        CHECK((readBytes= read(fd, buf, fileSize < MAX_READ-1 ? fileSize : MAX_READ-1)));
        CHECK(write(fd_dst, buf, readBytes));
        fileSize -= readBytes;
    }

    CHECK(close(fd_dst))

    return 0;
}

int delete(int fd, const char *name, const char *file){

    int offset, fd_2, readBytes, fileSize;
    unsigned char buf[MAX_READ];

    if((offset = search(fd,name))==-1){  //arxi
        printf("File not found in DB\n");
        return -1;
    }

    CHECK(lseek(fd, offset, SEEK_SET));

    //find entry's end
    CHECK((fd_2 = open(file, O_RDWR)));

    CHECK(lseek(fd_2, offset,SEEK_SET));

    CHECK((readBytes=read(fd_2, buf, FILENAMELEN)));

    CHECK((readBytes = read(fd_2, buf, (size_t)*buf)));  //Diabazei to onoma

    CHECK(read(fd_2, &fileSize, MAXFILESIZE));  //Diabazei to megethos tou arxeiou

    CHECK(lseek(fd_2, fileSize, SEEK_CUR));  //Telos arxeiou

    //overwrite start from end until end reaches EOF
    while((readBytes = read(fd_2,buf, MAX_READ-1))){
        CHECK(readBytes);
        CHECK(write(fd, buf, readBytes));
    }

    //truncate
    CHECK(readBytes = lseek(fd, 0, SEEK_CUR));  //Neo EOF ths DB

    CHECK(ftruncate(fd_2,readBytes));

    CHECK(close(fd_2))

    return 0;
}