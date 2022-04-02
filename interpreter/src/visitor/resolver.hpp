#ifndef _RESOLVER_HPP_
#define _RESOLVER_HPP_
#include <memory>
#include <stack>
#include <string>
#include <map>
#include <span>
#include <vector>
#include "visitor.hpp"
#include "interpreter.hpp"
#include "../token.hpp"

enum class FunctionType
{
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD,
};

enum class ClassType
{
    NONE,
    CLASS,
    SUBCLASS,
};

class Resolver : public Visitor
{
public:
    explicit Resolver(Interpreter &);
    void resolve(const std::vector<std::shared_ptr<Stmt>> &);
    void resolve(const std::shared_ptr<const Stmt>);
    void resolve(const std::shared_ptr<const Expr>);

    std::shared_ptr<Object> visitVariable(const std::shared_ptr<const Variable>) override;
    std::shared_ptr<Object> visitAssign(const std::shared_ptr<const Assign>) override;
    std::shared_ptr<Object> visitBinary(const std::shared_ptr<const Binary>) override;
    std::shared_ptr<Object> visitCall(const std::shared_ptr<const Call>) override;
    std::shared_ptr<Object> visitGrouping(const std::shared_ptr<const Grouping>) override;
    std::shared_ptr<Object> visitLiteral(const std::shared_ptr<const Literal>) override;
    std::shared_ptr<Object> visitLogical(const std::shared_ptr<const Logical>) override;
    std::shared_ptr<Object> visitUnary(const std::shared_ptr<const Unary>) override;
    std::shared_ptr<Object> visitGet(const std::shared_ptr<const Get>) override;
    std::shared_ptr<Object> visitSet(const std::shared_ptr<const Set>) override;
    std::shared_ptr<Object> visitThis(const std::shared_ptr<const This>) override;
    std::shared_ptr<Object> visitSuper(const std::shared_ptr<const Super>) override;
    void visitExpression(const std::shared_ptr<const Expression>) override;
    void visitIf(const std::shared_ptr<const If>) override;
    void visitBlock(const std::shared_ptr<const Block>) override;
    void visitFunction(const std::shared_ptr<const Function>) override;
    void visitPrint(const std::shared_ptr<const Print>) override;
    void visitReturn(const std::shared_ptr<const Return>) override;
    void visitWhile(const std::shared_ptr<const While>) override;
    void visitVar(const std::shared_ptr<const Var>) override;
    void visitBreak(const std::shared_ptr<const Break>) override;
    void visitClass(const std::shared_ptr<const Class>) override;

private:
    void resolveLocal(const std::shared_ptr<const Expr>, const Token &);
    void resolveFunction(const std::shared_ptr<const Function>, FunctionType);
    void declare(const Token &);
    void define(const Token &);
    void beginScope();
    void endScope();
    Interpreter &interpreter;
    std::vector<std::map<std::string, bool>> scopes;
    FunctionType currentFunction = FunctionType::NONE;
    ClassType currentClass = ClassType::NONE;
    int loopScopeDepth = 0;
};
#endif