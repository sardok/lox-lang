#include <iostream>
#include <string>
#include "token.hpp"

std::ostream &operator<<(std::ostream &os, const Token &token)
{
    std::stringstream ss;
    ss << std::string(token.start, token.length);
    ss << "[line" << token.line << "]";
    os << ss.str();
    return os;
}