#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

    }

    return 0;
}
