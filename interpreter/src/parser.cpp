#include <span>
#include "stmt/class.hpp"
#include <string>
#include <iostream>
#include <vector>
#include "parser.hpp"
#include "token.hpp"
#include "expr/binary.hpp"
#include "expr/unary.hpp"
#include "expr/literal.hpp"
#include "expr/grouping.hpp"
#include "expr/variable.hpp"
#include "expr/assign.hpp"
#include "expr/logical.hpp"
#include "expr/call.hpp"
#include "expr/get.hpp"
#include "expr/set.hpp"
#include "expr/this.hpp"
#include "expr/super.hpp"
#include "stmt/expression.hpp"
#include "stmt/print.hpp"
#include "stmt/var.hpp"
#include "stmt/block.hpp"
#include "stmt/if.hpp"
#include "stmt/while.hpp"
#include "stmt/break.hpp"
#include "stmt/function.hpp"
#include "stmt/return.hpp"
#include "stmt/class.hpp"
#include "object/string_object.hpp"
#include "object/number_object.hpp"
#include "object/nil_object.hpp"
#include "object/bool_object.hpp"
#include "exception/parse_error.hpp"

using std::make_shared;
using std::move;
using std::shared_ptr;

Parser::Parser(std::span<Token> tokens) : tokens{tokens}, current{0}
{
}

std::vector<shared_ptr<Stmt>> Parser::parse()
{
    std::vector<shared_ptr<Stmt>> stmts;

    while (!isAtEnd())
    {
        auto stmt = declaration();
        stmts.push_back(stmt);
    }

    return stmts;
}

shared_ptr<Stmt> Parser::declaration()
{
    if (match(TokenType::CLASS))
        return classDeclaration();

    if (match(TokenType::FUN))
        return funDeclaration("function");

    if (match(TokenType::VAR))
        return varDeclaration();

    return statement();
}

shared_ptr<Stmt> Parser::classDeclaration()
{
    auto name = consume(TokenType::IDENTIFIER, "Expect class name.");
    shared_ptr<Variable> super;

    if (match(TokenType::LESSER))
    {
        consume(TokenType::IDENTIFIER, "Expect super class name.");
        super = make_shared<Variable>(previous());
    }

    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");
    std::vector<shared_ptr<Function>> methods;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        auto method = dynamic_pointer_cast<Function>(funDeclaration("method"));
        methods.push_back(std::move(method));
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return make_shared<Class>(move(name), move(super), move(methods));
}

shared_ptr<Stmt> Parser::funDeclaration(std::string kind)
{
    auto name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if (parameters.size() > 255)
                throw ParseError(peek(), "Can't have more than 255 parameters");

            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    auto body = dynamic_pointer_cast<Block>(blockStatement());
    return make_shared<Function>(move(name), move(parameters), body->stmts);
}

shared_ptr<Stmt> Parser::varDeclaration()
{
    auto token = consume(TokenType::IDENTIFIER, "Expect variable name.");

    shared_ptr<Expr> initializer;
    if (match(TokenType::EQUAL))
        initializer = expression();

    consume(TokenType::SEMICOLON, "Expect semicolon.");
    return make_shared<Var>(move(token), move(initializer));
}

shared_ptr<Stmt> Parser::statement()
{
    if (match(TokenType::IF))
        return ifStatement();

    if (match(TokenType::PRINT))
        return printStatement();

    if (match(TokenType::RETURN))
        return returnStatement();

    if (match(TokenType::WHILE))
        return whileStatement();

    if (match(TokenType::FOR))
        return forStatement();

    if (match(TokenType::BREAK))
        return breakStatement();

    if (match(TokenType::LEFT_BRACE))
        return blockStatement();

    return expressionStatement();
}

shared_ptr<Stmt> Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after if.");
    auto expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if expression.");

    auto thenBranch = statement();
    shared_ptr<Stmt> elseBranch;
    if (match(TokenType::ELSE))
    {
        elseBranch = statement();
    }

    return make_shared<If>(move(expr), move(thenBranch), move(elseBranch));
}

shared_ptr<Stmt> Parser::printStatement()
{
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return make_shared<Print>(move(expr));
}

shared_ptr<Stmt> Parser::returnStatement()
{
    auto token = previous();
    shared_ptr<Expr> expr;

    if (!check(TokenType::SEMICOLON))
        expr = expression();

    consume(TokenType::SEMICOLON, "Expect ';' after return.");
    return make_shared<Return>(move(token), move(expr));
}

shared_ptr<Stmt> Parser::whileStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after while.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while.");
    auto body = statement();
    return make_shared<While>(condition, body);
}

shared_ptr<Stmt> Parser::forStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after for.");
    std::shared_ptr<Stmt> initializer;

    if (match(TokenType::SEMICOLON))
    {
    }
    else if (match(TokenType::VAR))
        initializer = varDeclaration();
    else
        initializer = expressionStatement();

    std::shared_ptr<Expr> condition;
    if (!check(TokenType::SEMICOLON))
        condition = expression();

    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::shared_ptr<Expr> increment;
    if (!check(TokenType::RIGHT_PAREN))
        increment = expression();

    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    auto body = statement();
    if (increment)
        body = make_shared<Block>(std::vector<shared_ptr<Stmt>>{body, make_shared<Expression>(increment)});

    if (!condition)
        condition = make_shared<Literal>(make_shared<BoolObject>(true));

    body = make_shared<While>(condition, body);
    if (initializer)
        body = make_shared<Block>(std::vector<shared_ptr<Stmt>>{initializer, body});

    return body;
}

shared_ptr<Stmt> Parser::blockStatement()
{
    std::vector<shared_ptr<Stmt>> stmts;
    while (!isAtEnd() && !check(TokenType::RIGHT_BRACE))
    {
        auto stmt = declaration();
        stmts.push_back(move(stmt));
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' to end block.");
    return make_shared<Block>(move(stmts));
}

shared_ptr<Stmt> Parser::expressionStatement()
{
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return make_shared<Expression>(expr);
}

shared_ptr<Stmt> Parser::breakStatement()
{
    consume(TokenType::SEMICOLON, "Expect ';' after break.");
    auto token = previous();
    return make_shared<Break>(move(token));
}

shared_ptr<Expr> Parser::expression()
{
    return assignment();
}

shared_ptr<Expr> Parser::assignment()
{
    auto expr = logicalOr();

    if (match(TokenType::EQUAL))
    {
        auto value = assignment();
        auto variableExpr = dynamic_pointer_cast<Variable>(expr);
        if (variableExpr)
        {
            auto name = variableExpr->name;
            return make_shared<Assign>(move(name), move(value));
        }

        auto getExpr = dynamic_pointer_cast<Get>(expr);
        if (getExpr != nullptr)
            return make_shared<Set>(getExpr->expr, getExpr->name, move(value));

        auto equals = previous();
        throw ParseError(equals, "Invalid assignment target.");
    }

    return expr;
}

shared_ptr<Expr> Parser::logicalOr()
{
    auto expr = logicalAnd();
    while (match(TokenType::OR))
    {
        auto token = previous();
        auto right = logicalAnd();
        expr = make_shared<Logical>(expr, move(token), move(right));
    }

    return expr;
}

shared_ptr<Expr> Parser::logicalAnd()
{
    auto expr = equality();
    while (match(TokenType::AND))
    {
        auto token = previous();
        auto right = equality();
        expr = make_shared<Logical>(expr, move(token), move(right));
    }

    return expr;
}

shared_ptr<Expr> Parser::equality()
{
    auto expr = comparison();

    TokenType signs[]{
        TokenType::EQUAL_EQUAL,
        TokenType::BANG_EQUAL};

    while (match(signs))
    {
        auto op = previous();
        auto right = comparison();
        expr = make_shared<Binary>(expr, move(op), move(right));
    }

    return expr;
}

shared_ptr<Expr> Parser::comparison()
{
    auto expr = term();

    TokenType signs[]{
        TokenType::GREATER,
        TokenType::GREATER_EQUAL,
        TokenType::LESSER,
        TokenType::LESSER_EQUAL};

    while (match(signs))
    {
        auto op = previous();
        auto right = term();
        expr = make_shared<Binary>(expr, move(op), move(right));
    }

    return expr;
}

shared_ptr<Expr> Parser::term()
{
    auto expr = factor();

    TokenType signs[]{
        TokenType::PLUS,
        TokenType::MINUS};

    while (match(signs))
    {
        auto op = previous();
        auto right = factor();
        expr = make_shared<Binary>(expr, move(op), move(right));
    }

    return expr;
}

shared_ptr<Expr> Parser::factor()
{
    auto expr = unary();

    TokenType signs[]{
        TokenType::STAR,
        TokenType::SLASH};

    while (match(signs))
    {
        auto op = previous();
        auto right = unary();
        expr = make_shared<Binary>(expr, move(op), move(right));
    }

    return expr;
}

shared_ptr<Expr> Parser::unary()
{
    TokenType signs[]{
        TokenType::MINUS,
        TokenType::BANG};

    if (match(signs))
    {
        auto op = previous();
        auto right = unary();
        return make_shared<Unary>(move(op), move(right));
    }

    return call();
}

shared_ptr<Expr> Parser::call()
{
    auto expr = primary();
    while (true)
    {
        if (match(TokenType::LEFT_PAREN))
        {
            expr = finishCall(expr);
        }
        else if (match(TokenType::DOT))
        {
            auto name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = make_shared<Get>(expr, name);
        }
        else
            break;
    }

    return expr;
}

shared_ptr<Expr> Parser::primary()
{
    if (match(TokenType::TRUE))
        return make_shared<Literal>(make_shared<BoolObject>(true));

    if (match(TokenType::FALSE))
        return make_shared<Literal>(make_shared<BoolObject>(false));

    if (match(TokenType::NIL))
        return make_shared<Literal>(make_shared<NilObject>());

    if (match(TokenType::NUMBER))
    {
        auto token = previous();
        if (!token.literal.has_value())
            throw ParseError{token, "Missing value"};

        auto literal = token.literal.value();
        return make_shared<Literal>(make_shared<NumberObject>(literal));
    }

    if (match(TokenType::STRING))
    {
        auto token = previous();
        if (!token.literal.has_value())
            throw ParseError{token, "Missing value"};

        auto literal = token.literal.value();
        return make_shared<Literal>(make_shared<StringObject>(literal));
    }

    if (match(TokenType::SUPER))
    {
        auto token = previous();
        consume(TokenType::DOT, "Expect '.' after 'super'.");
        auto method = consume(TokenType::IDENTIFIER, "Expect superclass method name.");
        return make_shared<Super>(token, method);
    }

    if (match(TokenType::THIS))
        return make_shared<This>(previous());

    if (match(TokenType::IDENTIFIER))
        return make_shared<Variable>(previous());

    if (match(TokenType::LEFT_PAREN))
    {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return make_shared<Grouping>(expr);
    }

    throw ParseError(peek(), "Unexpected token");
}

shared_ptr<Call> Parser::finishCall(shared_ptr<Expr> callee)
{
    std::vector<shared_ptr<Expr>> arguments;
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if (arguments.size() >= 255)
                throw ParseError(peek(), "Can't have more than 254 arguments.");

            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    auto token = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return make_shared<Call>(std::move(callee), std::move(token), std::move(arguments));
}

bool Parser::match(std::span<TokenType> tokenTypes)
{
    for (auto &tokenType : tokenTypes)
    {
        if (match(tokenType))
        {
            return true;
        }
    }

    return false;
}

bool Parser::match(TokenType tokenType)
{
    if (check(tokenType))
    {
        auto _ = advance();
        return true;
    }

    return false;
}

bool Parser::check(TokenType tokenType)
{
    if (isAtEnd())
        return false;

    return peek().type == tokenType;
}

Token Parser::advance()
{
    if (!isAtEnd())
        current++;
    return previous();
}

Token Parser::previous()
{
    return tokens[current - 1];
}

bool Parser::isAtEnd()
{
    return peek().type == TokenType::EOF_;
}

Token &Parser::peek()
{
    return tokens[current];
}

Token Parser::consume(TokenType tokenType, std::string errorMsg)
{
    if (check(tokenType))
        return advance();

    throw ParseError(tokenType, errorMsg);
}

void Parser::synchronize()
{
    advance();

    while (!isAtEnd())
        if (previous().type != TokenType::SEMICOLON)
            return;

    switch (peek().type)
    {
    case TokenType::CLASS:
    case TokenType::FUN:
    case TokenType::VAR:
    case TokenType::FOR:
    case TokenType::IF:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::RETURN:
        return;
    default:
        break;
    }

    advance();
}