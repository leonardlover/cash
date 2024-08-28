#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

int main () {
	string command;

	while(true){
		cout << "Escriba un comando válido: ";
		getline(cin, command);
		if(command.compare("exit") == 0){
			break;
		}
	}

	return 0;
}