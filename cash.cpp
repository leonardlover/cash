#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h> 
#include <sys/wait.h>

int main(void)
{
    std::string buffer, word;

    while (true) {
        std::cout << "cash:$ ";
        std::getline(std::cin, buffer);
        std::stringstream stream(buffer);
        std::vector<std::vector<std::string> > commands;

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

        for (int i = 0; i < commands.size(); i++) {
            std::cout << commands[i].size() << std::endl;
            for (int j = 0; j < commands[i].size(); j++) {
                std::cout << commands[i][j] << " ";
            }
            std::cout << std::endl;
        }
        
        int num_children = commands.size();
        pid_t pid;
        int pipes[2];
        pipe(pipes);

        for(int i=0; i<num_children; i++){
            pid = fork();

            if(pid == -1){
                std::cout << "Error en la creación del proceso hijo " << i << std::endl;
                exit(1);
            }
            else if(pid == 0){

                if(i%2 == 0){
                    // Resultado es escrito en pipe
                    dup2(pipes[1], 1);
                }
                else{
                    // Entrada es extraída de pipe1
                    dup2(pipes[0], 0);
                }

                close(pipes[0]);
                close(pipes[1]);

                int size = commands[0].size();
                char *args[size + 1];
                args[size] = NULL;

                for(int j=0; j<size; j++){
                    args[j] = commands[i][j].data();
                }

                execvp(*args, args);
            }
        }

        close(pipes[0]);
        close(pipes[1]);

        for(int i=0; i<num_children; i++){
            wait(NULL);
        }
    }

    return 0;
}
