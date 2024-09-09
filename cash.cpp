#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <filesystem>
#include <signal.h>

#include "lexer.cpp"
#include "childprocess.cpp"

void changeDir(char *newDir){
    int change = chdir(newDir);
    if(change != 0){
        std::cout << newDir << ": " << std::strerror(errno) << std::endl;
    }
}

void prompt(){
    std::cout << "\033[1;36m";
    	
    char *username = getlogin();
    char *directory = get_current_dir_name();

    if (username) {
        std::cout << username;
    } else {
        std::cout << "user";
    }

    std::cout << "@CASH\033[0m:";

    if(directory){
        std::cout << directory;
    } else {
        std::cout << "some directory";
    }

    free(directory);

    std::cout <<  "$ ";
}

//Message to output in alarm must be global for now
std::vector<std::string> out_message; 

void sigHandler(int sig){
    if(sig == SIGALRM){
        std::cout << "Recordatorio: ";
        for(int i = 0; i < out_message.size(); ++i){
                std::cout << " " << out_message[i];
        }
        std::cout << std::endl;
	prompt();
	std::fflush(stdout);

    }		
}


int main(void)
{

    signal(SIGALRM, sigHandler);	

    std::string buffer, word, prevFavs, cash_prompt;

    std::vector<std::string> favorite;
    std::ifstream readFavs("misfavoritos.txt");

    bool cmdError;
    bool equalFavFlag;
    char* favsDir;
    favsDir = get_current_dir_name();

    while(getline(readFavs, prevFavs)){
        favorite.push_back(prevFavs);
    }

    readFavs.close();

    char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

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

        bool emptyCmd = false;
        for (int i = 0; i < commands.size(); i++) {
            if (commands[i].empty()) {
                std::cerr << "error: empty command" << std::endl;
                emptyCmd = true;
                break;
            }
        }

        if (emptyCmd)
            continue;

        cmdError = false;
        equalFavFlag = false;

        if (commands.size() == 1 && commands.back().empty())
            continue;

        if (commands[0].size() > 0 && commands[0][0] == "exit")
            break;

        if(commands[0][0] == "cd"){
            if (commands.front().size() == 1) {
                changeDir(homedir);
            } else {
                changeDir(commands[0][1].data());
            }

            continue;
        }

        if(commands[0][0] == "favs" && commands[0].size() > 1) {
            if(commands[0][1] == "crear"){
                favsDir = commands[0][2].data();

                if(fork() == 0){
                    changeDir(favsDir);
                    std::ofstream Fav("misfavoritos.txt");
                    for(int i = 0; i < favorite.size(); i++){
                        Fav << favorite[i] << '\n';
                    } 
                    Fav.close();
                    exit(0);
                }
                waitForChildren(1, &cmdError);
                continue;
            }


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

                char* auxDir = get_current_dir_name();
                cout << auxDir << '\n';
                changeDir(favsDir);
                std::ifstream readFavs("misfavoritos.txt");

                while(getline(readFavs, prevFavs)){
                    favorite.push_back(prevFavs);
                }

                readFavs.close();

                changeDir(auxDir);

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
                bool gotOne = false;

                for(int i = 2; i < commands[0].size(); i++){
                    for(int j = 0; j < commands[0][i].size(); j++){
                        if(commands[0][i][j] >= 48 && commands [0][i][j] <= 58 && !gotOne){
                            num1 = commands[0][i][j];
                            gotOne = true;
                            continue;
                        }

                        if(commands[0][i][j] >= 48 && gotOne && commands [0][i][j] <= 58){
                            num2 = commands[0][i][j];
                        }
                    }
                }

                num1 -= 49;
                num2 -= 49;

                if(num1 >= 0 && num2 >= 0 && num1 < favorite.size() && num2 < favorite.size()){
                    if(num1 == num2){
                        favorite.erase(favorite.begin()+num1);
                    } else {
                        if(num1 > num2)
                            favorite.erase(favorite.begin()+num2, favorite.begin()+(num1+1));
                        else 
                            favorite.erase(favorite.begin()+num1, favorite.begin()+(num2+1));
                    }
                }
                continue;
            }

            if(commands[0][1] == "guardar"){
                char* auxDir = get_current_dir_name();
                changeDir(favsDir);

                std::ofstream Fav("misfavoritos.txt");
                for(int i = 0; i < favorite.size(); i++){
                    Fav << favorite[i] << '\n';
                } 
                Fav.close();

                changeDir(auxDir);

                continue;
            }

            /*if(commands[0][2] == "ejecutar"){
                if(favorite.size() < commands[0][1][0] - 48 || (int)commands[0][1][0] <= 0){
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
                continue;
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
                message.push_back(commands[0][i]);
            }
                
           	out_message = message;
	       	alarm(stoi(commands[0][2]));	
                continue;	    
        }
		
        int numChildren = commands.size();

        if(numChildren == 1){
            createAndRunOneChild(commands, &cmdError);
        }

        else if(numChildren > 1){
            createAndRunMultipleChildren(commands, numChildren, &cmdError);
        }

        waitForChildren(numChildren, &cmdError);

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
