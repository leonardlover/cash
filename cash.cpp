#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h> 
#include <sys/wait.h>
#include <fstream>
#include <filesystem>
#include <linux/limits.h>
#include <signal.h>

#include "lexer.cpp"

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

void createPipes(int* pipes, int num_children){
    for(int i=0; i<num_children - 1; i++){
        pipe(pipes + 2*i);
    }
}

void pipeAssignation(int* pipes, int i, int num_children){
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
}

void closePipes(int* pipes, int num_children){
    for(int i=0; i<2*(num_children - 1); i++){
        close(pipes[i]);
    }
}

void prompt(){
    std::cout << "\033[1;36m";

    if(getlogin() == NULL){
        std::cout << "user";
    }
    else{
        std::cout << getlogin();
    }

    std::cout << "@CASH\033[0m:" << getDir() <<  "$ ";
}

void sigHandler(int sig){
    if(sig = SIGALRM){
	std::cout << "Alarma" << std::endl;
    }		
}

void recordatorio(int time, std::vector<std::string> message){
	alarm(time);
	//TODO: This is NOT a valid implementation. Need for a way to output custom input message on SIGALARM signal
	for(int i = 0; i < message.size(); ++i){
		std::cout << " " << message[i]; 
	}
	std::cout << std::endl;
	
}

int main(void)
{

    signal(SIGALRM, sigHandler);	

    std::string buffer, word;
    std::vector<std::string> favorite;
    bool cmdError;
    std::ifstream readFavs("misfavoritos.txt");

    /* Aqui iría la inyección de misfavoritos.txt a favorite, un indice por cada linea
    si es que supiese cómo hacerla */
    

    while (true) {
        prompt();
        std::getline(std::cin, buffer);
        lexer lex(buffer);
        std::vector<std::string> tokens;
        try {
            tokens = lex.tokenize();
        } catch (const std::logic_error &e) {
            std::cerr << "error: " << e.what() << std::endl;
            continue;
        }

        std::vector<std::vector<std::string> > commands;
        commands.push_back({});
        for (int i = 0; i < tokens.size(); i++) {
            if (tokens[i] != "|")
                commands.back().push_back(tokens[i]);
            else
                commands.push_back({});
        }
        cmdError = false;

        if (commands[0].size() > 0 && commands[0][0] == "exit")
            break;

        if(commands[0][0] == "cd"){
            changeDir(commands[0][1].data());
            continue;
	    }

        if(commands[0][0] == "set" && commands[0][1] == "recordatorio"){
            if(commands[0].size() < 4){
            std::cout << "Error: Argumentos faltantes" << std::endl;
         
            continue;
            }
            
            try{
                int rec_time = stoi(commands[0][2]);
            }
            catch(const std::invalid_argument& ia){
                std::cout << "Argumento de tiempo inválido" << std::endl;
                continue;
            }
            
            std::vector<std::string> message;
            
            for(int i = 3; i < commands[0].size(); ++i){
		if(i ==3){
			if(commands[0][i].std::string::length() < 3){
				std::cout << "Argumento inválido" << std::endl;
				continue;
			}
	        	if(commands[0][i][0] != '\"' ){
				std::cout << "Argumento inválido: Comilla faltante" << std::endl;
				continue;
			}
			commands[0][i].erase(0,1); 
		        	
		}
	        if(i == commands[0].size() - 1){
			if(commands[0][i][commands[0][i].std::string::length()-1] != '\"' ){
				std::cout << "Argumento inválido: Comilla faltante" << std::endl;
				continue;
			}
			commands[0][i].erase(commands[0][i].std::string::length()-1, 1);
			
		}	
                message.push_back(commands[0][i]);
            }
                
            recordatorio(stoi(commands[0][2]), message); 
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
            createPipes(pipes, num_children);

            for(int i=0; i<num_children; i++){
                pid = fork();

                if(pid == -1){
                    std::cout << "Error en la creación del proceso hijo " << i << std::endl;
                    cmdError = true;
                    exit(1);
                }
                else if(pid == 0){
                    pipeAssignation(pipes, i, num_children);
                    closePipes(pipes, num_children);
                    runCommand(commands[i], cmdError);
                }
            }

            closePipes(pipes, num_children);
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
