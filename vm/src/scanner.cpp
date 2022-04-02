#include <cstring>
#include "scanner.hpp"

using std::move;

Token Scanner::scanToken()
{
    skipWhitespace();
    start = current;

    if (isAtEnd())
        return makeToken(TokenType::EOF_);

    auto c = advance();
    if (std::isalpha(c))
        return identifier();
    if (std::isdigit(c))
        return number();

    switch (c)
    {
    case '(':
        return makeToken(TokenType::LEFT_PAREN);
    case ')':
        return makeToken(TokenType::RIGHT_PAREN);
    case '{':
        return makeToken(TokenType::LEFT_BRACE);
    case '}':
        return makeToken(TokenType::RIGHT_BRACE);
    case ';':
        return makeToken(TokenType::SEMICOLON);
    case ',':
        return makeToken(TokenType::COMMA);
    case '.':
        return makeToken(TokenType::DOT);
    case '-':
        return makeToken(TokenType::MINUS);
    case '+':
        return makeToken(TokenType::PLUS);
    case '/':
        return makeToken(TokenType::SLASH);
    case '*':
        return makeToken(TokenType::STAR);
    case '!':
        return makeToken(
            match('=')
                ? TokenType::BANG_EQUAL
                : TokenType::BANG);
    case '=':
        return makeToken(
            match('=')
                ? TokenType::EQUAL_EQUAL
                : TokenType::EQUAL);
    case '<':
        return makeToken(
            match('=')
                ? TokenType::LESS_EQUAL
                : TokenType::LESS);
    case '>':
        return makeToken(
            match('=')
                ? TokenType::GREATER_EQUAL
                : TokenType::GREATER);
    case '"':
        return string();
    default:
        break;
    }

    return errorToken("Unexpected character.");
}

Token Scanner::makeToken(TokenType type)
{
    return Token{type, start, current - start, line};
}

bool Scanner::isAtEnd()
{
    auto endPtr = source.data() + source.size();
    return current == endPtr;
}

Token Scanner::errorToken(const char *msg)
{
    Token token;
    token.type = TokenType::ERROR;
    token.start = msg;
    token.length = strlen(msg);
    token.line = line;
    return token;
}

char Scanner::advance()
{
    return *current++;
}

bool Scanner::match(char expected)
{
    if (isAtEnd())
        return false;

    if (*current != expected)
        return false;

    current++;
    return true;
}

void Scanner::skipWhitespace()
{
    for (;;)
    {
        auto c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            advance();
            line++;
            break;
        case '/':
            if (peekNext() == '/')
            {
                while (peek() != '\n' && !isAtEnd())
                    advance();
                break;
            }
            else
                return;
        default:
            return;
        }
    }
}

char Scanner::peek()
{
    return *current;
}

char Scanner::peekNext()
{
    if (isAtEnd())
        return '\0';
    return *(current + 1);
}

Token Scanner::string()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd())
        return errorToken("Unterminated string.");

    advance(); // The closing quote.
    return makeToken(TokenType::STRING);
}

Token Scanner::number()
{
    while (!isAtEnd() && std::isdigit(peek()))
        advance();

    if (!isAtEnd() && peek() == '.' && std::isdigit(peekNext()))
        advance();

    while (!isAtEnd() && std::isdigit(peek()))
        advance();

    return makeToken(TokenType::NUMBER);
}

Token Scanner::identifier()
{
    while (!isAtEnd() && std::isalnum(peek()))
        advance();

    return makeToken(identifierType());
}

TokenType Scanner::identifierType()
{
    switch (*start)
    {
    case 'a':
        return checkKeyword(1, 2, "nd", TokenType::AND);
    case 'c':
        return checkKeyword(1, 4, "lass", TokenType::CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", TokenType::ELSE);
    case 'f':
        if (current - start > 1)
        {
            switch (*(start + 1))
            {
            case 'a':
                return checkKeyword(2, 3, "lse", TokenType::FALSE);
            case 'o':
                return checkKeyword(2, 1, "r", TokenType::FOR);
            case 'u':
                return checkKeyword(2, 1, "n", TokenType::FUN);
            }
        }
        break;
    case 'i':
        return checkKeyword(1, 1, "f", TokenType::IF);
    case 'n':
        return checkKeyword(1, 2, "il", TokenType::NIL);
    case 'o':
        return checkKeyword(1, 1, "r", TokenType::OR);
    case 'p':
        return checkKeyword(1, 4, "rint", TokenType::PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", TokenType::RETURN);
    case 's':
        return checkKeyword(1, 4, "uper", TokenType::SUPER);
    case 't':
        if (current - start > 1)
        {
            switch (*(start + 1))
            {
            case 'h':
                return checkKeyword(2, 2, "is", TokenType::THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", TokenType::TRUE);
            }
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", TokenType::VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TokenType::WHILE);
    }

    return TokenType::IDENTIFIER;
}

TokenType Scanner::checkKeyword(int offset, int len, const char *dst, TokenType type)
{
    if (current - start == offset + len && memcmp(start + offset, dst, len) == 0)
    {
        return type;
    }
    return TokenType::IDENTIFIER;
}