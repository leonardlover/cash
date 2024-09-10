#ifndef CD
#define CD

#include <cstring>

#include <stdlib.h>
#include <unistd.h> 

void changeDir(char *newDir){
    if(chdir(newDir) != 0){
        std::cerr << newDir << ": " << std::strerror(errno) << std::endl;
    }
}

#endif
