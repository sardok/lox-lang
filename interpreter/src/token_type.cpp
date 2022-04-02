#include <iostream>
#include "token.hpp"

std::ostream &operator<<(std::ostream &os, const TokenType &tokenType)
{
    switch (tokenType)
    {
    case TokenType::LEFT_PAREN:
        return os << "(";
    case TokenType::RIGHT_PAREN:
        return os << ")";
    case TokenType::LEFT_BRACE:
        return os << "{";
    case TokenType::RIGHT_BRACE:
        return os << "}";
    case TokenType::COMMA:
        return os << ",";
    case TokenType::DOT:
        return os << ".";
    case TokenType::MINUS:
        return os << "-";
    case TokenType::PLUS:
        return os << "+";
    case TokenType::SEMICOLON:
        return os << ";";
    case TokenType::SLASH:
        return os << "/";
    case TokenType::STAR:
        return os << "*";
    case TokenType::AND:
        return os << "and";
    case TokenType::BANG:
        return os << "!";
    default:
        return os << "unrecognized token";
    }
}