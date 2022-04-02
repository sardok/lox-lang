#include <iostream>
#include <optional>
#include "scanner.hpp"

Scanner::Scanner(std::string &source)
    : source{source}
{
    keywords.emplace("and", TokenType::AND);
    keywords.emplace("class", TokenType::CLASS);
    keywords.emplace("false", TokenType::FALSE);
    keywords.emplace("fun", TokenType::FUN);
    keywords.emplace("for", TokenType::FOR);
    keywords.emplace("while", TokenType::WHILE);
    keywords.emplace("if", TokenType::IF);
    keywords.emplace("else", TokenType::ELSE);
    keywords.emplace("nil", TokenType::NIL);
    keywords.emplace("or", TokenType::OR);
    keywords.emplace("print", TokenType::PRINT);
    keywords.emplace("return", TokenType::RETURN);
    keywords.emplace("super", TokenType::SUPER);
    keywords.emplace("this", TokenType::THIS);
    keywords.emplace("true", TokenType::TRUE);
    keywords.emplace("var", TokenType::VAR);
    keywords.emplace("break", TokenType::BREAK);
}

std::vector<Token> Scanner::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }

    addToken(TokenType::EOF_);
    return tokens;
}

void Scanner::scanToken()
{
    auto c = advance();
    switch (c)
    {
    case '(':
        addToken(TokenType::LEFT_PAREN);
        break;
    case ')':
        addToken(TokenType::RIGHT_PAREN);
        break;
    case '{':
        addToken(TokenType::LEFT_BRACE);
        break;
    case '}':
        addToken(TokenType::RIGHT_BRACE);
        break;
    case ',':
        addToken(TokenType::COMMA);
        break;
    case '.':
        addToken(TokenType::DOT);
        break;
    case '-':
        addToken(TokenType::MINUS);
        break;
    case '+':
        addToken(TokenType::PLUS);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '!':
        match('=')
            ? addToken(TokenType::BANG_EQUAL)
            : addToken(TokenType::BANG);
        break;
    case '=':
        match('=')
            ? addToken(TokenType::EQUAL_EQUAL)
            : addToken(TokenType::EQUAL);
        break;
    case '<':
        match('=')
            ? addToken(TokenType::LESSER_EQUAL)
            : addToken(TokenType::LESSER);
        break;
    case '>':
        match('=')
            ? addToken(TokenType::GREATER_EQUAL)
            : addToken(TokenType::GREATER);
        break;
    case '/':
        if (match('/'))
        {
            while (!isAtEnd() && peek() != '\n')
                advance();
        }
        else
        {
            addToken(TokenType::SLASH);
        }
        break;
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n':
        line++;
        break;
    case '"':
        string();
        break;
    default:
        if (isDigit(c))
            number();
        else if (isAlpha(c))
            identifier();
        else
            std::cout << line << "Unexpected character." << std::endl;
        break;
    }
}

char &Scanner::advance()
{
    return source.at(current++);
}

void Scanner::addToken(TokenType type)
{
    addToken(type, std::nullopt);
}

void Scanner::addToken(TokenType type, std::optional<std::string> literal)
{
    auto text = source.substr(start, current - start);
    Token token{type, text, literal, line};
    tokens.push_back(token);
}

bool Scanner::match(char c)
{
    if (isAtEnd())
        return false;

    if (peek() != c)
        return false;

    advance();
    return true;
}

char Scanner::peek()
{
    if (isAtEnd())
        return '\0';

    return source.at(current);
}

char Scanner::peekNext()
{
    if (current + 1 >= source.length())
        return '\0';

    return source.at(current + 1);
}

void Scanner::string()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            line++;

        advance();
    }

    if (isAtEnd())
    {
        std::cout << "Unterminated string." << std::endl;
        return;
    }

    advance();
    auto literal = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, literal);
}

bool Scanner::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNum(char c)
{
    return isAlpha(c) || isDigit(c);
}

void Scanner::number()
{
    char c;
    while (isDigit(peek()))
        advance();

    if (peek() == '.' && isDigit(peekNext()))
    {
        advance();

        while (isDigit(peek()))
            advance();
    }

    auto num = source.substr(start, current - start);
    addToken(TokenType::NUMBER, num);
}

void Scanner::identifier()
{
    while (isAlphaNum(peek()))
        advance();

    auto text = source.substr(start, current - start);
    auto token =
        keywords.count(text)
            ? keywords.at(text)
            : TokenType::IDENTIFIER;
    addToken(token);
}

bool Scanner::isAtEnd()
{
    return current >= source.length();
}