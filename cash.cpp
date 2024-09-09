#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <fstream>
#include <filesystem>
#include <linux/limits.h>
#include <signal.h>

#include "lexer.cpp"


void changeDir(char *newDir){
    int change = chdir(newDir);
    if(change != 0){
        std::cout << newDir << ": " << std::strerror(errno) << std::endl;
    }
}

void runCommand(std::vector<std::string> command, bool *error){
    int size = command.size();
    char *args[size + 1];
    args[size] = NULL;

    for(int j=0; j<size; j++){
        args[j] = command[j].data();
    }

    if(execvp(*args, args)==-1){
        *error = true;
        std:: cout << *args << " :Comando no encontrado" << std::endl;   
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

    char *username = getlogin();

    if (username) {
        std::cout << username;
    } else {
        std::cout << "user";
    }

    std::cout << "@CASH\033[0m:";

    char *directory = get_current_dir_name();

    if(directory){
        std::cout << directory;
    } else {
        std::cout << "some directory";
    }

    std::cout <<  "$ ";
}

//Message to output in alarm must be global for now
std::vector<std::string> out_message;

void sigHandler(int sig){
    if(sig = SIGALRM){
        std::cout << "Recordatorio: ";
        for(int i = 0; i < out_message.size(); ++i){
                std::cout << " " << out_message[i];
        }
        std::cout << std::endl;
    }		
}


int main(void)
{

    signal(SIGALRM, sigHandler);	

    std::string buffer, word, prevFavs;

    std::vector<std::string> favorite;
    std::ifstream readFavs("misfavoritos.txt");

    bool cmdError;
    bool equalFavFlag;

    while(getline(readFavs, prevFavs)){
        favorite.push_back(prevFavs);
    }

    readFavs.close();

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
        equalFavFlag = false;

        if (commands[0].size() > 0 && commands[0][0] == "exit")
            break;

        if(commands[0][0] == "cd"){
            changeDir(commands[0][1].data());
            continue;
	    }

        if(commands[0][0] == "favs" && commands[0].size() > 1) {
            if(commands[0][1] == "mostrar"){
                if(favorite.empty()) {
                    std::cout << "Aún no hay favoritos." << '\n';
                    continue;
                } else {
                    for(int i = 0; i < favorite.size(); i++){
                        std::cout << i+1 << ". " << favorite[i] << '\n';
                    }
                    continue;
                }
            }

            if(commands[0][1] == "buscar"){
                for(int i = 0; i < favorite.size(); i++){
                    if(favorite[i].find(commands[0][2])!=std::string::npos){
                        std::cout << i+1 << ". " << favorite[i] << '\n';
                    }
                }
                continue;
            }

            if(commands[0][1] == "borrar"){
                if(favorite.empty()){
                    std::cout << "Aún no hay favoritos." << '\n';
                    continue;
                } else {
                favorite.clear();
                continue;
                } 
            }

            if(commands[0][1] == "cargar"){
                favorite.clear();
                std::ifstream readFavs("misfavoritos.txt");

                while(getline(readFavs, prevFavs)){
                    favorite.push_back(prevFavs);
                }
                readFavs.close();

                if(favorite.empty()) {
                    std::cout << "Aún no hay favoritos." << '\n';
                    continue;
                } else {
                    for(int i = 0; i < favorite.size(); i++){
                        std::cout << i+1 << ". " << favorite[i] << '\n';
                    }
                    continue;
                }
            }

            if(commands[0][1] == "eliminar"){
                int num1 = 0;
                int num2 = 0;
                for(int i = 2; i < commands[0].size(); i++){
                    for(int j = 0; j < commands[0][i].size(); j++){
                        if(commands[0][i][j] - 48 > 0){
                            num1 = commands[0][i][j] - 48;
                        }

                        if(commands[0][i][j] - 48 > 0 && num1 > 0){
                            num2 = commands[0][i][j] - 48;
                        }
                    }
                }

                if(num1 > 0 && num2 > 0 && num1 < favorite.size() && num2 < favorite.size()){
                    if(num1 > num2)
                        favorite.erase(favorite.begin()+(num2-1), favorite.begin()+(num1-1));
                    else 
                        favorite.erase(favorite.begin()+(num1-1), favorite.begin()+(num2-1));
                }
                continue;
            }

            if(commands[0][1] == "guardar"){
                std::ofstream Fav("misfavoritos.txt");
                for(int i = 0; i < favorite.size(); i++){
                    Fav << favorite[i] << '\n';
                } 
                Fav.close();
                continue;
            }

            /*if(commands[0][2] == "ejecutar"){
                if(favorite.size() < (int)commands[0][1][0] || (int)commands[0][1][0] <= 0){
                    std::cout << "Número no válido para ejecutar." << '\n';
                } else {
                    std::string favTokens;

                    for(int i = 0; i < favorite[((int)commands[0][1][0]) - 1].size(); i++){
                        favTokens.push_back(favorite[(commands[0][1][0]) - 1][i]);
                    }

                    std::vector<std::vector<std::string> > commands;
                    commands.push_back({});
                    for (int i = 0; i < favTokens.size(); i++) {
                        if (favTokens[i] != '|')
                            commands.back().push_back(favTokens[i]);
                        else
                            commands.push_back({});
                    }
                } 
            }*/


        }

        if(commands[0][0] == "set" && commands[0][1] == "recordatorio"){
            if(commands[0].size() < 4){
            std::cout << "Error: Argumentos faltantes" << std::endl;
            cmdError = true;
         
            continue;
            }
            
            try{
                int rec_time = stoi(commands[0][2]);
            }
            catch(const std::invalid_argument& ia){
                std::cout << "Argumento de tiempo inválido" << std::endl;
                cmdError = true;
                continue;
            }
            
            std::vector<std::string> message;
            
            for(int i = 3; i < commands[0].size(); ++i){
		/*    
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
	*/	
                message.push_back(commands[0][i]);
            }
                
           	out_message = message;
	       	alarm(stoi(commands[0][2]));	
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
                runCommand(commands[0], &cmdError);       
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
                    runCommand(commands[i], &cmdError);
                }
            }

            closePipes(pipes, num_children);
        }

        for(int i=0; i<num_children; i++){
            wait(NULL);
        }

        if (!cmdError){
            for(int i = 0; i < favorite.size(); i++){
                if(favorite[i].compare(buffer) == 0){
                    equalFavFlag = true;
                    break;
                }
            }
        }
        if(!equalFavFlag && !cmdError) favorite.push_back(buffer);
    }

    return 0;
}
