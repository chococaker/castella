#include <iostream>

#include "attacks.h"
#include "uci.h"

int main() {
    std::cout << std::fixed << std::setprecision(5);

    choco::attacks::initAttacks();

    choco::UciInstance inst;

    std::string input;
    std::cin >> std::ws;

    while (true) {
        if (!std::getline(std::cin, input) && std::cin.eof()) {
            input = "quit";
        }

        inst.processLine(input);

        if (input == "quit") break;
    }
}
