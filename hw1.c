/*Giorgos Dimos
 *06/05/2020
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "db.h"

#define MAX_ARGS 3
#define MAX_STR 255

#ifndef CHECK
#define CHECK(sinartisi)\
    if((sinartisi) == -1){\
        fprintf(stderr,"errno: %d at line: %d\n", errno, __LINE__);\
        return -1;\
    }
#endif

static int mystrtok(const char *str, char delim, char *tokenizedStr[], int arraySize);

/****************************************************************************************************************************/

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

    CHECK(close(fd))

    return 1;
}

/****************************************************************************************************************************/

static int mystrtok(const char *str, char delim, char *tokenizedStr[], int arraySize){
    int i=0, j;
    size_t len;
    char tok[strlen(str)];

    if(!*str || arraySize<=0){  //invalid input
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
        j=0;
        while(str[len]!=delim && str[len]!='\0'){ //find length of substring
            if(str[len]=='\\' && str[len+1]==delim){  //detect delim in name and skip
                len++;
            }
            tok[j++] = str[len++];
        }
        tok[j] = '\0';

        tokenizedStr[i] = (char *)malloc(j+1);  //malloc memory for new string
        if(!tokenizedStr[i]){
            for (int j=0;j< i; j++){
                free(tokenizedStr[j]);
            }
            fprintf(stderr,"Malloc failed\n");
            return -1;
        }
        strcpy(tokenizedStr[i], tok);
        
        str+=len;
        i++;
    }
}