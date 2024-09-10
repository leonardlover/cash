#ifndef FAVS
#define FAVS

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <cstring> 

#include <unistd.h>

#include "cd.cpp"
#include "childprocess.cpp"
#include "lexer.cpp"

void crearFavs(std::string newFavDir, char* &favsDirPointer, std::string newFileName, std::string *favsFileName, bool *error){
    if(newFavDir.empty()){
        std::cerr << "Falta un directorio" << std::endl;
        *error = true;
        return;
    }
    if(newFileName.empty()){
        std::cerr << "Falta el nombre del archivo" << std::endl;
        *error = true;
        return;
    }

    *favsFileName = newFileName;

		if (favsDirPointer) {
        free(favsDirPointer);
    }

    favsDirPointer = strdup(newFavDir.c_str());

    if(fork() == 0){
        changeDir(favsDirPointer);

        std::ifstream readFavs;

        try{
            readFavs.open(*favsFileName);

            if(!readFavs.is_open()){
                throw std::ifstream::failure("File does not exit");
            }

            readFavs.close();
            std::cout << *favsFileName << " ya existe en la dirección " << favsDirPointer << std::endl; 
            std::cout << "Use favs cargar para extraer los favoritos del archivo." << std::endl;
        }
        catch(const std::ifstream::failure& e){
            std::ofstream Fav(*favsFileName);

            readFavs.open(*favsFileName);

            if(readFavs.is_open()){
                std::cout << *favsFileName << " fue creado con éxito en la dirección " << favsDirPointer << std::endl;
                readFavs.close();
            }
            else{
                std::cout << "Hubo un error en la creación del archivo " << *favsFileName << std::endl;
            }

        }

        std::exit(EXIT_SUCCESS);
    }

    waitForChildren(1, error);
}

void mostrarFavs(std::vector<std::string> favorite){
    if(favorite.empty()) {
        std::cout << "Aún no hay favoritos" << std::endl;
    } else {
        for(std::size_t i = 0; i < favorite.size(); i++){
            std::cout << i+1 << ". " << favorite[i] << std::endl;
        }
    }
}

void buscarFavs(std::vector<std::string> favorite, std::string command){
    for(std::size_t i = 0; i < favorite.size(); i++){
        if(favorite[i].find(command)!=std::string::npos){
            std::cout << i+1 << ". " << favorite[i] << std::endl;
        }
    }
}

void borrarFavs(std::vector<std::string> *favorite){
    if(favorite->empty()){
        std::cout << "Aún no hay favoritos " << std::endl;
    } else {
        favorite->clear();
    }
}

void cargarFavs(char* favsDirPointer, std::string favsFileName, std::vector<std::string> *favorite, std::string prevFavs, bool *error){
    favorite->clear();

    if(fork() == 0){
        changeDir(favsDirPointer);

        std::ifstream readFavs;

        try{
            readFavs.open(favsFileName);

            if(!readFavs.is_open()){
                throw std::ifstream::failure("File does not exist");
            }

            while(getline(readFavs, prevFavs)){
                favorite->push_back(prevFavs);
            }

            readFavs.close();

            if(favorite->empty()){
                std::cout << "Aún no hay favoritos" << std::endl;
            }
            else{
                std::size_t i = 0;
                for(const auto& str: *favorite){
                    std::cout << i++ << ". " << str << std::endl;
                }
            }
        }
        catch(const std::ifstream::failure& e){
            std::cout << "No existe el archivo " << favsFileName << " en la dirección " << favsDirPointer << std::endl;
        }

        std::exit(EXIT_SUCCESS);
    }

    waitForChildren(1, error);
}

void guardarFavs(char* favsDirPointer, std::string favsFileName, std::vector<std::string> favorite, bool *error){
    if(fork() == 0){
        changeDir(favsDirPointer);
        std::ofstream Fav;

        try{
            Fav.open(favsFileName);

            if(!Fav.is_open()){
                throw std::ifstream::failure("File does not exit");
            }

            for(std::size_t i = 0; i < favorite.size(); i++){
                Fav << favorite[i] << '\n';
            }

            Fav.close();
            std::cout << "Comandos guardados con éxito en el archivo " << favsFileName << " en la dirección " << favsDirPointer << std::endl;
        }
        catch(const std::ifstream::failure& e){
            std::cout << "No existe el archivo " << favsFileName << " en la dirección " << favsDirPointer << std::endl;
            std::cout << "Use favs crear para generar el archivo en que desea cargar los comandos" << std::endl;
        }

        std::exit(EXIT_FAILURE);
    }

    waitForChildren(1, error);
}

void eliminarFavs(std::vector<std::string> *favorite, std::vector<std::vector<std::string>> commands){
    std::string num1Str;
    std::string num2Str;
    long long num1 = 0;
    long long num2 = 0;
    bool gotOne = false;

    for(std::size_t i = 2; i < commands[0].size(); i++){
        for(std::size_t j = 0; j < commands[0][i].size(); j++){
            if(commands[0][i][j] >= 48 && commands [0][i][j] <= 57 && !gotOne){
                num1Str.push_back(commands[0][i][j]);
                for(std::size_t k = 1; k < commands[0][i].size(); k++){
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
                for(std::size_t k = 1; k < commands[0][i].size(); k++){
                    if(commands[0][i][k] >= 48 && commands [0][i][k] <= 58) 
                        num2Str.push_back(commands[0][i][k]);
                    else 
                        break;
                }
            }
        }
    }

    long long powerCounter = num1Str.size() - 1;
    for(std::size_t i = 0; i < num1Str.size(); i++){
        num1 += ((num1Str[i] - 49) * pow(10, powerCounter));
        powerCounter--;
    }

    powerCounter = num2Str.size() - 1;
    for(std::size_t i = 0; i < num2Str.size(); i++){
        num2 += ((num2Str[i] - 49) * pow(10, powerCounter));
        powerCounter--;
    }

    if(num1 >= 0 && num2 >= 0 && num1 < favorite->size() && num2 < favorite->size()){
        if(num1 == num2){
            favorite->erase(favorite->begin()+num1);
        } else {
            if(num1 > num2)
                favorite->erase(favorite->begin()+num2, favorite->begin()+(num1+1));
            else 
                favorite->erase(favorite->begin()+num1, favorite->begin()+(num2+1));
        }
    }
}

void ejecutarFavs(std::vector<std::vector<std::string>> commands, std::vector<std::vector<std::string>> *commandsFav, std::vector<std::string> favorite, bool *favoriteExec, bool *execError){
    std::string strToInt;
    long long exec = -1;

    for(std::size_t i = 0; i < commands[0][1].size(); i++){
        if(commands[0][1][i] >= 48 && commands[0][1][i] <= 57){
            strToInt.push_back(commands[0][1][i]);
        }
    }


    if(!strToInt.empty()){
        exec = 0;
        long long powerCounter = strToInt.size()-1;
        for(std::size_t i = 0; i < strToInt.size(); i++){
            exec += ((strToInt[i] - 49) * pow(10, i));
            powerCounter--;
        }
    }

    if(favorite.size() < exec || exec == -1){
        std::cerr << "Número no válido para ejecutar." << std::endl;
        *execError = true;
        return;
    } else {
        lexer lexFav(favorite[exec]);
        std::vector<std::string> tokensFav;
        try {
            tokensFav = lexFav.tokenize();
        } catch (const std::logic_error &e) {
            std::cerr << "error: " << e.what() << std::endl;
            *execError = true;
            return;
        }
        commandsFav->push_back({});
        for (std::size_t i = 0; i < tokensFav.size(); i++) {
            if (tokensFav[i] != "|")
                commandsFav->back().push_back(tokensFav[i]);
            else
                commandsFav->push_back({});
        }
        *favoriteExec = true;
    } 	
}

#endif
