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
#include <cmath>

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
    bool favoriteExec;
    std::string favsDir;
    char* favsDirPointer;
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
        favoriteExec = false;
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
        std::vector<std::vector<std::string> > commandsFav;
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
                favsDir = commands[0][2];
                favsDirPointer = favsDir.data();
                char* auxDir = get_current_dir_name();

                changeDir(favsDirPointer);

                std::ifstream readFavs("misfavoritos.txt");
                while(getline(readFavs, prevFavs)){
                    favorite.push_back(prevFavs);
                }
                readFavs.close();

                if(favorite.empty()){
                    std::ofstream Fav("misfavoritos.txt");
                    for(int i = 0; i < favorite.size(); i++){
                        Fav << favorite[i] << '\n';
                    } 
                    Fav.close();
                }

                changeDir(auxDir);

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
                favsDirPointer = favsDir.data();

                changeDir(favsDirPointer);

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
                std::string num1Str;
                std::string num2Str;
                int num1 = 0;
                int num2 = 0;
                bool gotOne = false;

                for(int i = 2; i < commands[0].size(); i++){
                    for(int j = 0; j < commands[0][i].size(); j++){
                        if(commands[0][i][j] >= 48 && commands [0][i][j] <= 57 && !gotOne){
                            num1Str.push_back(commands[0][i][j]);
                            for(int k = 1; k < commands[0][i].size(); k++){
                                if(commands[0][i][k] >= 48 && commands [0][i][k] <= 58) 
                                    num1Str.push_back(commands[0][i][k]);
                                else 
                                    break;
                            }
                            gotOne = true;
                            continue;
                        }

                        if(commands[0][i][j] >= 48 && commands [0][i][j] <= 57 && gotOne){
                            num2Str.push_back(commands[0][i][j]);
                            for(int k = 1; k < commands[0][i].size(); k++){
                                if(commands[0][i][k] >= 48 && commands [0][i][k] <= 58) 
                                    num2Str.push_back(commands[0][i][k]);
                                else 
                                    break;
                            }
                        }
                    }
                }
                
                for(int i = 0; i < num1Str.size(); i++){
                    num1 += ((num1Str[i] - 49) * pow(10, i));
                }

                for(int i = 0; i < num2Str.size(); i++){
                    num2 += ((num2Str[i] - 49) * pow(10, i));
                }

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
                favsDirPointer = favsDir.data();

                changeDir(favsDirPointer);

                std::ofstream Fav("misfavoritos.txt");
                for(int i = 0; i < favorite.size(); i++){
                    Fav << favorite[i] << '\n';
                } 
                Fav.close();

                changeDir(auxDir);

                continue;
            }

            if(commands[0][2] == "ejecutar"){
                std::string strToInt;
                int exec = -1;

                for(int i = 0; i < commands[0][1].size(); i++){
                    if(commands[0][1][i] >= 48 && commands[0][1][i] <= 57){
                        strToInt.push_back(commands[0][1][i]);
                    }
                }
                
                if(!strToInt.empty()){
                    exec = 0;
                    for(int i = 0; i < strToInt.size(); i++){
                        exec += ((strToInt[i] - 49) * pow(10, i));
                    }
                    cout << exec << '\n';
                }

                if(favorite.size() < exec || exec == -1){
                    std::cout << "Número no válido para ejecutar." << '\n';
                    continue;
                } else {
                    cout << favorite[exec] << '\n';
                    lexer lexFav(favorite[exec]);
                    std::vector<std::string> tokensFav;
                    try {
                        tokensFav = lexFav.tokenize();
                    } catch (const std::logic_error &e) {
                        std::cerr << "error: " << e.what() << std::endl;
                        continue;
                    }
                    commandsFav.push_back({});
                    for (int i = 0; i < tokensFav.size(); i++) {
                        if (tokensFav[i] != "|")
                            commandsFav.back().push_back(tokensFav[i]);
                        else
                            commandsFav.push_back({});
                    }
                    favoriteExec = true;
                } 
            }
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
		
        if(favoriteExec){
            int numChildren = commandsFav.size();

            if(numChildren == 1){
                createAndRunOneChild(commandsFav, &cmdError);
            }

            else if(numChildren > 1){
                createAndRunMultipleChildren(commandsFav, numChildren, &cmdError);
            }

            waitForChildren(numChildren, &cmdError);
        } else {
            int numChildren = commands.size();

            if(numChildren == 1){
                createAndRunOneChild(commands, &cmdError);
            }

            else if(numChildren > 1){
                createAndRunMultipleChildren(commands, numChildren, &cmdError);
            }

            waitForChildren(numChildren, &cmdError);
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
