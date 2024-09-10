#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
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
#include "favs.cpp"
#include "cd.cpp"

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

    bool cmdError;
    bool equalFavFlag;
    bool favoriteExec;
    char* favsDirPointer = get_current_dir_name();
    std::string favsFileName = "misfavoritos.txt";

    std::ifstream readFavs(favsFileName);

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
                if(commands[0].size() <= 3){
                    cout << "Error: Argumentos faltantes." << std::endl;
                    continue;
                } else {
                    // PENDIENTE AGREGAR EL NOMBRE DEL ARCHIVO
                    crearFavs(commands[0][2], favsDirPointer, commands[0][3], &favsFileName, &cmdError);
                    continue;
                }
            }

            if(commands[0][1] == "mostrar"){
                mostrarFavs(favorite);
                continue;
            }

            if(commands[0][1] == "buscar"){
                if(commands[0].size() == 2){
                    cout << "Error: Argumentos faltantes." << std::endl;
                } else {
                    buscarFavs(favorite, commands[0][2]);
                }
                continue;
            }

            if(commands[0][1] == "borrar"){
                borrarFavs(&favorite);
                continue;
            }

            if(commands[0][1] == "cargar"){
                cargarFavs(favsDirPointer, favsFileName, &favorite, prevFavs, &cmdError);
                continue;
            }

            if(commands[0][1] == "eliminar"){
                eliminarFavs(&favorite, commands);
                continue;
            }

            if(commands[0][1] == "guardar"){
                guardarFavs(favsDirPointer, favsFileName, favorite, &cmdError);
                continue;
            }

            if(commands[0][2] == "ejecutar"){
                bool execError = false;
                ejecutarFavs(commands, &commandsFav, favorite, &favoriteExec, &execError);
                if(execError){
                    continue;
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

    free(favsDirPointer);
    return 0;
}
