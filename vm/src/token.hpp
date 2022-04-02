#ifndef _TOKEN_HPP_
#define _TOKEN_HPP_
#include <iostream>
#include <sstream>
enum class TokenType : std::uint32_t
{
    // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,
    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,
    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FOR,
    FUN,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    ERROR,
    EOF_
};

TokenType operator+(const TokenType, int);

struct Token
{
    Token() = default;
    Token(TokenType type, const char *start, long length, int line)
        : type{type}, start{start}, length{length}, line{line}
    {
    }

    TokenType type;
    const char *start;
    long length;
    int line;
};

std::ostream &operator<<(std::ostream &os, const Token &token);
#endif