#include "db.h"

static int search(int fd, const char *fileToFind);

/*********************************  Import File to Database  **************************************************************/

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

/****************************  Find file in Database  ****************************************************/

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

/***************************************  Export file from Database  ********************************************************/

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

/***********************************  Delete file from Database  **************************************************/

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

/****************************************  Search for file in Database  **********************************************/

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