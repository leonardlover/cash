#ifndef CD
#define CD

#include <unistd.h> 
#include <stdlib.h>

void changeDir(char *newDir){
    if(chdir(newDir) != 0){
        std::cout << newDir << ": " << std::strerror(errno) << std::endl;
    }
}

#endif