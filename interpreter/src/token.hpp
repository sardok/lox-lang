#ifndef _TOKEN_HPP_
#define _TOKEN_HPP_
#include <string>
#include <iostream>
#include <sstream>
#include <optional>
#include "token_type.hpp"

class Token
{
public:
    const TokenType type;
    const std::string lexeme;
    const std::optional<std::string> literal;
    const int line;

    Token(TokenType type, std::string lexeme, std::optional<std::string> literal, int line)
        : type{type}, lexeme{lexeme}, literal{literal}, line{line}
    {
    }

    std::string toString()
    {
        std::stringstream ss;
        ss << type << " " << lexeme;
        if (literal.has_value()) ss << " " << literal.value(); 
        return ss.str();
    }
};
#endif