#ifndef _SCANNER_HPP_
#define _SCANNER_HPP_
#include <string>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>
#include "token.hpp"

class Scanner
{
public:
    explicit Scanner(std::string &source);
    std::vector<Token> scanTokens();

private:
    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, std::optional<std::string> literal);
    char &advance();
    bool match(char c);
    char peek();
    char peekNext();
    bool isAtEnd();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNum(char c);
    void string();
    void number();
    void identifier();

    int start = 0;
    int current = 0;
    int line = 1;
    std::string source;
    std::vector<Token> tokens;
    std::unordered_map<std::string, TokenType> keywords;
};
#endif