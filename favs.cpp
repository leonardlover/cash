#ifndef FAVS
#define FAVS

#include <string>
#include <vector>
#include <fstream>
#include <exception>

#include "cd.cpp"
#include "childprocess.cpp"

void crearFavs(std::string *favsDir, std::string newFavDir, char* favsDirPointer, bool *error){
	if(newFavDir.empty()){
		cout << "Falta un directorio." << '\n';
		*error = true;
		return;
	}
	*favsDir = newFavDir;
	favsDirPointer = favsDir->data();

	if(fork() == 0){
		changeDir(favsDirPointer);

		std::ifstream readFavs;
		
		try{
			readFavs.open("misfavoritos.txt");

			if(!readFavs.is_open()){
				throw std::ifstream::failure("File does not exit");
			}

			readFavs.close();
			std::cout << "misfavoritos.txt ya existe en la dirección " << *favsDir << std::endl; 
			std::cout << "Use favs cargar para extraer los favoritos del archivo." << std::endl;
		}
		catch(const std::ifstream::failure& e){
			std::ofstream Fav("misfavoritos.txt");
			std::cout << "misfavoritos.txt fue creado con éxito en la dirección " << *favsDir << std::endl;
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

#endif