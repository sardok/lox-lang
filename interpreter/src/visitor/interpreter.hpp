#ifndef _INTERPRETER_HPP_
#define _INTERPRETER_HPP_
#include <memory>
#include <map>
#include <span>
#include "../object/object.hpp"
#include "../expr/expr.hpp"
#include "../stmt/stmt.hpp"
#include "../token.hpp"
#include "visitor.hpp"
#include "../environment.hpp"

class LoxCallable;

class Interpreter : public Visitor
{
public:
    explicit Interpreter();
    void interpret(const std::vector<std::shared_ptr<Stmt>> &);
    void interpret(const std::shared_ptr<Stmt> &);
    void resolve(const std::shared_ptr<const Expr>, int depth);
    std::shared_ptr<Object> visitBinary(const std::shared_ptr<const Binary>) override;
    std::shared_ptr<Object> visitGrouping(const std::shared_ptr<const Grouping>) override;
    std::shared_ptr<Object> visitLiteral(const std::shared_ptr<const Literal>) override;
    std::shared_ptr<Object> visitUnary(const std::shared_ptr<const Unary>) override;
    std::shared_ptr<Object> visitVariable(const std::shared_ptr<const Variable>) override;
    std::shared_ptr<Object> visitAssign(const std::shared_ptr<const Assign>) override;
    std::shared_ptr<Object> visitLogical(const std::shared_ptr<const Logical>) override;
    std::shared_ptr<Object> visitCall(const std::shared_ptr<const Call>) override;
    std::shared_ptr<Object> visitGet(const std::shared_ptr<const Get>) override;
    std::shared_ptr<Object> visitSet(const std::shared_ptr<const Set>) override;
    std::shared_ptr<Object> visitThis(const std::shared_ptr<const This>) override;
    std::shared_ptr<Object> visitSuper(const std::shared_ptr<const Super>) override;
    void visitExpression(const std::shared_ptr<const Expression>) override;
    void visitPrint(const std::shared_ptr<const Print>) override;
    void visitVar(const std::shared_ptr<const Var>) override;
    void visitBlock(const std::shared_ptr<const Block>) override;
    void visitIf(const std::shared_ptr<const If>) override;
    void visitWhile(const std::shared_ptr<const While>) override;
    void visitBreak(const std::shared_ptr<const Break>) override;
    void visitFunction(const std::shared_ptr<const Function>) override;
    void visitReturn(const std::shared_ptr<const Return>) override;
    void visitClass(const std::shared_ptr<const Class>) override;
    void executeBlock(const std::vector<std::shared_ptr<Stmt>> &, std::shared_ptr<Environment>);

private:
    void registerGlobals();
    std::shared_ptr<Object> evaluate(const shared_ptr<Expr>);
    bool isTruthy(const std::shared_ptr<Object> &) const;
    bool isEqual(const std::shared_ptr<Object> &, const std::shared_ptr<Object> &) const;
    void throwOperandsMismatch(const Token &token, const std::shared_ptr<Object> &, const std::shared_ptr<Object> &) const;
    void throwIfArgumentsMismatch(const Token &, const std::shared_ptr<LoxCallable> &, std::span<std::shared_ptr<Object>>) const;
    std::shared_ptr<Object> lookupVariable(const Token &, const std::shared_ptr<const Expr>);
    std::map<const std::shared_ptr<const Expr>, int> locals;
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
};
#endif