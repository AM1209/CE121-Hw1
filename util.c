#include"util.h"

int mystrtok(const char *str, const char delim, char *tokenizedStr[], const int arraySize){
    int i=0;
    size_t len;

    while(1){
        
        if(*str=='\0')  //reached end of string return number of tokens
            return i;
    
        if(i == arraySize)  //reached Array limits, string not fully tokenized, return failure
            return 0;

        while(*str==delim) //discard space character
            str++; 

        len=0;  
        while(str[len]!=delim && str[len]!='\0') //find length of substring
            len++;

        tokenizedStr[i] = (char *)malloc(len+1);  //malloc memory for new string
        strncpy(tokenizedStr[i], str, len);
        tokenizedStr[i][len]='\0';

        str+=len;
        i++;
    }
}