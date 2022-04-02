#ifndef _LOX_HPP_
#define _LOX_HPP_
#include <string>

class Lox
{
public:
    Lox(std::string args[], int len);
    Lox() = default;
    void runFile(std::string filename);
    void runPrompt();
    void run(std::string source);

private:
    bool hadError{false};
};
#endif