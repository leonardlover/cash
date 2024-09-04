#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h> 
#include <sys/wait.h>
#include <fstream>
#include <filesystem>
#include <linux/limits.h>

char* getDir(){
    char cwd[PATH_MAX];
    return getcwd(cwd, sizeof(cwd));
}

void changeDir(char *newDir){
    int change = chdir(newDir);
    if(change != 0){
        std::cout << newDir << ": " << std::strerror(errno) << std::endl;
    }
}

void runCommand(std::vector<std::string> command, bool &error){
    int size = command.size();
    char *args[size + 1];
    args[size] = NULL;

    for(int j=0; j<size; j++){
        args[j] = command[j].data();
    }

    if(execvp(*args, args)==-1){
        std:: cout << *args << " :Comando no encontrado" << std::endl;
        error = true;
        exit(1);
    }
}

int main(void)
{
    std::string buffer, word;
    std::vector<std::string> favorite;
    bool cmdError;

    while (true) {
        std::cout << "\033[1;36m" << getlogin() << "@CASH\033[0m:" << getDir() <<  "$ ";
        std::getline(std::cin, buffer);
        std::stringstream stream(buffer);
        std::vector<std::vector<std::string> > commands;
        cmdError = false;

        if (buffer == "exit")
            break;

        while (stream >> word) {
            if (commands.empty()) {
                commands.push_back({});
            }

            if (word == "|") {
                commands.push_back({});
            } else {
                commands.back().push_back(word);
            }
        }

        /*
        for (int i = 0; i < commands.size(); i++) {
            std::cout << commands[i].size() << std::endl;
            for (int j = 0; j < commands[i].size(); j++) {
                std::cout << commands[i][j] << " ";
            }
            std::cout << std::endl;
        }
        */

        if(commands[0][0] == "cd"){
            changeDir(commands[0][1].data());
            continue;
        }
        
        int num_children = commands.size();
        pid_t pid;

        if(num_children == 1){
            pid = fork();

            if(pid == -1){
                std::cout << "Error en la creación del proceso hijo" << std::endl;
                cmdError = true;
                exit(1);
            }

            else if(pid == 0){
                runCommand(commands[0], cmdError);       
            }
        }

        // Múltiples hijos requieren pipes
        else if(num_children > 1){
            int pipes[2*(num_children - 1)];
            for(int i=0; i<num_children - 1; i++){
                pipe(pipes + 2*i);
            }

            for(int i=0; i<num_children; i++){
                pid = fork();

                if(pid == -1){
                    std::cout << "Error en la creación del proceso hijo " << i << std::endl;
                    cmdError = true;
                    exit(1);
                }
                else if(pid == 0){

                    if(i == 0){
                        // Primer proceso recibe input por entrada estándar o argumento y escribe en primer pipe
                        dup2(pipes[1], 1);
                    }
                    else if(i == num_children - 1){
                        // Último proceso recibe entrada por último pipe y escribe en salida estándar (si es que escribe)
                        dup2(pipes[2*i - 2], 0);
                    }
                    else{
                        // Cualquier otro proceso intermedio recibe entrada por pipe anterior y escribe en pipe siguiente
                        dup2(pipes[2*i - 2], 0);
                        dup2(pipes[2*i + 1], 1);
                    }

                    for(int j=0; j<2*(num_children - 1); j++){
                        close(pipes[j]);
                    }

                    runCommand(commands[i], cmdError);
                }

            }

            for(int j=0; j<2*(num_children - 1); j++){
                close(pipes[j]);
            }
        }

        for(int i=0; i<num_children; i++){
            wait(NULL);
        }
        if (!cmdError) favorite.push_back(buffer);
    }

    std::ofstream Fav("misfavoritos.txt");
    for(int i = 0; i < favorite.size(); i++){
        Fav << favorite[i] << '\n';
    } 
    Fav.close();

    return 0;
}
