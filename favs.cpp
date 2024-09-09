#ifndef FAVS
#define FAVS

#include <string>
#include <vector>
#include <fstream>
#include <exception>

#include "cd.cpp"
#include "childprocess.cpp"

void crearFavs(std::string newFavDir, char* favsDirPointer, bool *error){
	if(newFavDir.empty()){
		cout << "Falta un directorio." << '\n';
		*error = true;
		return;
	}
	favsDirPointer = newFavDir.data();

	if(fork() == 0){
		changeDir(favsDirPointer);

		std::ifstream readFavs;
		
		try{
			readFavs.open("misfavoritos.txt");

			if(!readFavs.is_open()){
				throw std::ifstream::failure("File does not exit");
			}

			readFavs.close();
			std::cout << "misfavoritos.txt ya existe en la dirección " << favsDirPointer << std::endl; 
			std::cout << "Use favs cargar para extraer los favoritos del archivo." << std::endl;
		}
		catch(const std::ifstream::failure& e){
			std::ofstream Fav("misfavoritos.txt");
			std::cout << "misfavoritos.txt fue creado con éxito en la dirección " << favsDirPointer << std::endl;
		}

		exit(0);
	}

	waitForChildren(1, error);
}

void mostrarFavs(std::vector<std::string> favorite){
	if(favorite.empty()) {
		std::cout << "Aún no hay favoritos" << std::endl;
	} else {
		for(int i = 0; i < favorite.size(); i++){
			std::cout << i+1 << ". " << favorite[i] << std::endl;
		}
	}
}

void buscarFavs(std::vector<std::string> favorite, std::string command){
	for(int i = 0; i < favorite.size(); i++){
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
				int i = 0;
				for(const auto& str: *favorite){
					std::cout << i++ << ". " << str << std::endl;
				}
			}
		}
		catch(const std::ifstream::failure& e){
			std::cout << "No existe el archivo " << favsFileName << " en la dirección " << favsDirPointer << std::endl;
		}

		exit(0);
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

			for(int i = 0; i < favorite.size(); i++){
				Fav << favorite[i] << '\n';
			}

			Fav.close();
			std::cout << "Comandos guardados con éxito en el archivo " << favsFileName << " en la dirección " << favsDirPointer << std::endl;
		}
		catch(const std::ifstream::failure& e){
			std::cout << "No existe el archivo " << favsFileName << " en la dirección " << favsDirPointer << std::endl;
			std::cout << "Use favs crear para generar el archivo en que desea cargar los comandos" << std::endl;
		}

		exit(0);
	}

	waitForChildren(1, error);
}

#endif