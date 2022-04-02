#ifndef _SCANNER_HPP_
#define _SCANNER_HPP_
#include <string>
#include "token.hpp"

class Scanner
{
public:
    explicit Scanner(std::string &source) : source{source}, start{&source[0]}, current{&source[0]} {}
    Token scanToken();

private:
    bool isAtEnd();
    Token makeToken(TokenType);
    Token errorToken(const char *);
    char advance();
    bool match(char);
    void skipWhitespace();
    char peek();
    char peekNext();
    Token string();
    Token number();
    Token identifier();
    TokenType identifierType();
    TokenType checkKeyword(int, int, const char *, TokenType);
    std::string &source;
    char *start;
    char *current;
    int line = 1;
};
#endif