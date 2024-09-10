#ifndef CHILDPROCESS
#define CHILDPROCESS

#include <cstdlib>
#include <string>
#include <vector>

#include <syd/types.h>
#include <syd/wait.h>
#include <unistd.h>

void runCommand(std::vector<std::string> command){
    std::size_t size = command.size();
    char *args[size + 1];
    args[size] = NULL;

    for(std::size_t j=0; j<size; j++){
        args[j] = command[j].data();
    }

    if(execvp(*args, args) == -1){
        std:: cout << *args << ": Comando no encontrado" << std::endl;   
        exit(1);
    }
}

void createPipes(int* pipes, int numChildren){
    for(int i=0; i<numChildren - 1; i++){
        pipe(pipes + 2*i);
    }
}

void pipeAssignation(int* pipes, int i, int numChildren){
    if(i == 0){
        // Primer proceso recibe input por entrada estándar o argumento y escribe en primer pipe
        dup2(pipes[1], 1);
    }
    else if(i == numChildren - 1){
        // Último proceso recibe entrada por último pipe y escribe en salida estándar (si es que escribe)
        dup2(pipes[2*i - 2], 0);
    }
    else{
        // Cualquier otro proceso intermedio recibe entrada por pipe anterior y escribe en pipe siguiente
        dup2(pipes[2*i - 2], 0);
        dup2(pipes[2*i + 1], 1);
    }
}

void closePipes(int* pipes, int numChildren){
    for(std::size_t i=0; i<2*(numChildren - 1); i++){
        close(pipes[i]);
    }
}

void createAndRunOneChild(std::vector<std::vector<std::string>> commands, bool *error){
    pid_t pid;
    pid = fork();

    if(pid == -1){
        std::cout << "Error en la creación del proceso hijo" << std::endl;
        *error = true;
    }

    else if(pid == 0){
        runCommand(commands[0]);    
    }
}

void createAndRunMultipleChildren(std::vector<std::vector<std::string>> commands, int numChildren, bool *error){
    pid_t pid;
    int pipes[2*(numChildren - 1)];
    createPipes(pipes, numChildren);

    for(int i=0; i<numChildren; i++){
        pid = fork();

        if(pid == -1){
            std::cout << "Error en la creación del proceso hijo " << i << std::endl;
            *error = true;
        }
        else if(pid == 0){
            pipeAssignation(pipes, i, numChildren);
            closePipes(pipes, numChildren);
            runCommand(commands[i]);
        }
    }

    closePipes(pipes, numChildren);
}

void waitForChildren(int numChildren, bool *error){
    int status;

    for(std::size_t i=0; i<numChildren; i++){
        wait(&status);
        if(WEXITSTATUS(status) != 0){
            *error = true;
        }
    }
}

#endif
