#include <iostream>
#include <fstream>
#include <exception>
#include "expr/binary.hpp"
#include "expr/unary.hpp"
#include "expr/literal.hpp"
#include "expr/grouping.hpp"
#include "visitor/ast_printer.hpp"
#include "visitor/interpreter.hpp"
#include "lox.hpp"
#include "token.hpp"
#include "scanner.hpp"
#include "parser.hpp"

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    std::cout << "One filename must be given" << std::endl;
    return -1;
  }

  std::ifstream input(argv[1]);
  if (input.fail())
    throw std::runtime_error("Invalid file " + std::string{argv[1]});

  std::stringstream content;
  content << input.rdbuf();
  input.close();

  Lox lox;
  lox.run(content.str());
  return 0;
}