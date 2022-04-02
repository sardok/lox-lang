#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include "chunk.hpp"
#include "disassembler.hpp"
#include "vm.hpp"

void repl(Vm &vm)
{
    for (;;)
    {
        std::string line;
        std::cout << "> ";
        if (!(std::getline(std::cin, line)))
        {
            std::cout << std::endl;
            break;
        }
        vm.interpret(line);
    }
}

std::string readFile(char *filename)
{
    std::ifstream input(filename);
    if (input.fail())
        throw std::runtime_error("Invalid file " + std::string{filename});

    std::stringstream content;
    content << input.rdbuf();
    input.close();
    return content.str();
}

int main(int argc, char *argv[])
{
    Vm vm;
    if (argc == 1)
    {
        repl(vm);
    }
    else if (argc == 2)
    {
        auto content = readFile(argv[1]);
        vm.interpret(content);
    }
    else
    {
        std::cout << "Usage: vlox [filename]" << std::endl;
        std::exit(65);
    }

    return 0;
}