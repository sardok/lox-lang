#ifndef _RUNTIME_ERROR_HPP_
#define _RUNTIME_ERROR_HPP_
#include <exception>
#include <string>
#include <sstream>
#include "../token.hpp"

class RuntimeError : public std::exception
{
public:
    RuntimeError(const Token &token, std::string msg)
    {
        std::stringstream ss;
        ss << "[" << token.lexeme << "]";
        ss << msg;
        ss << "(line: " << token.line << ")";
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