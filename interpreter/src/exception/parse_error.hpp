#ifndef _PARSE_ERROR_HPP_
#define _PARSE_ERROR_HPP_
#include <string>
#include <sstream>
#include <exception>
#include "../token.hpp"
#include "../token_type.hpp"

class ParseError : public std::exception
{
public:
    ParseError(TokenType &tokenType, std::string msg)
    {
        std::stringstream ss;
        ss << "(" << tokenType << ")";
        ss << msg;
        this->msg = ss.str();
    }

    ParseError(Token &token, std::string msg)
    {
        std::stringstream ss;
        ss << "[" << token.lexeme << "(" << token.type << ")]";
        ss << msg;
        this->msg = ss.str();
    }

    virtual const char *what() const throw()
    {
        return msg.c_str();
    }

private:
    std::string msg;
};

#endif