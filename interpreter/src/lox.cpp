#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "lox.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "visitor/resolver.hpp"
#include "visitor/interpreter.hpp"
#include "visitor/ast_printer.hpp"

using std::string;
namespace fs = std::filesystem;

Lox::Lox(string args[], int argc)
{
    if (argc > 1)
    {
        std::cout << "Too long argument list!" << std::endl;
        exit(65);
    }
}

void Lox::runFile(string filename)
{
    auto path = fs::path(filename);
    std::ifstream infile(path);
    std::stringstream buffer;
    buffer << infile.rdbuf();
}

void Lox::runPrompt()
{
    while (true)
    {
        string line;
        std::getline(std::cin, line);
        if (std::cin.eof())
            break;

        run(line);
    }
}

void Lox::run(string source)
{
    auto scanner = Scanner{source};
    auto tokens = scanner.scanTokens();

    if (hadError)
        return;

    Parser parser{tokens};
    auto stmts = parser.parse();

    Interpreter interpreter;
    Resolver resolver{interpreter};
    resolver.resolve(stmts);
    interpreter.interpret(stmts);
}