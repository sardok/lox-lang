#ifndef _PARSER_HPP_
#define _PARSER_HPP_
#include <span>
#include <memory>
#include "token.hpp"
#include "expr/expr.hpp"
#include "stmt/stmt.hpp"

class Call;

class Parser
{
public:
    explicit Parser(const std::span<Token> tokens);
    std::vector<std::shared_ptr<Stmt>> parse();

private:
    std::shared_ptr<Stmt> declaration();
    std::shared_ptr<Stmt> classDeclaration();
    std::shared_ptr<Stmt> funDeclaration(std::string);
    std::shared_ptr<Stmt> varDeclaration();
    std::shared_ptr<Stmt> statement();
    std::shared_ptr<Stmt> ifStatement();
    std::shared_ptr<Stmt> printStatement();
    std::shared_ptr<Stmt> returnStatement();
    std::shared_ptr<Stmt> whileStatement();
    std::shared_ptr<Stmt> forStatement();
    std::shared_ptr<Stmt> blockStatement();
    std::shared_ptr<Stmt> expressionStatement();
    std::shared_ptr<Stmt> breakStatement();
    std::shared_ptr<Expr> expression();
    std::shared_ptr<Expr> assignment();
    std::shared_ptr<Expr> logicalOr();
    std::shared_ptr<Expr> logicalAnd();
    std::shared_ptr<Expr> equality();
    std::shared_ptr<Expr> comparison();
    std::shared_ptr<Expr> term();
    std::shared_ptr<Expr> factor();
    std::shared_ptr<Expr> unary();
    std::shared_ptr<Expr> call();
    std::shared_ptr<Expr> primary();
    std::shared_ptr<Call> finishCall(std::shared_ptr<Expr>);
    bool match(std::span<TokenType> tokenTypes);
    bool match(TokenType tokenType);
    bool check(TokenType type);
    Token advance();
    Token previous();
    bool isAtEnd();
    Token &peek();
    Token consume(TokenType tokenType, std::string errorMsg);
    void synchronize();
    int current;
    const std::span<Token> tokens;
};
#endif