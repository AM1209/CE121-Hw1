#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#define CHECK(sinartisi)\
    if((sinartisi) == -1){\
        fprintf(stderr,"errno: %d at line: %d\n", errno, __LINE__);\
        return -1;\
    }\

#define MAX_READ 512
#define FILENAMELEN 1
#define MAXFILESIZE 8

int delete(int fd, const char *name, const char *file);
int export(int fd, const char *src, const char *dest);
int find(int fd, const char *fileToFind);
int import(int fd, const char *importFile);