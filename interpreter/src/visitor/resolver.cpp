#include <memory>
#include "resolver.hpp"
#include "../object/object.hpp"
#include "../stmt/stmt.hpp"
#include "../stmt/block.hpp"
#include "../stmt/function.hpp"
#include "../stmt/expression.hpp"
#include "../stmt/if.hpp"
#include "../stmt/print.hpp"
#include "../stmt/return.hpp"
#include "../stmt/while.hpp"
#include "../stmt/var.hpp"
#include "../stmt/break.hpp"
#include "../stmt/class.hpp"
#include "../expr/expr.hpp"
#include "../expr/variable.hpp"
#include "../expr/assign.hpp"
#include "../expr/binary.hpp"
#include "../expr/call.hpp"
#include "../expr/grouping.hpp"
#include "../expr/literal.hpp"
#include "../expr/logical.hpp"
#include "../expr/unary.hpp"
#include "../expr/get.hpp"
#include "../expr/set.hpp"
#include "../expr/this.hpp"
#include "../expr/super.hpp"
#include "../token.hpp"
#include "../exception/runtime_error.hpp"

using std::shared_ptr;

Resolver::Resolver(Interpreter &interpreter)
    : interpreter{interpreter}
{
}

void Resolver::resolve(const std::vector<std::shared_ptr<Stmt>> &stmts)
{
    for (const auto &stmt : stmts)
    {
        resolve(stmt);
    }
}

void Resolver::resolve(const shared_ptr<const Stmt> stmt)
{
    stmt->accept(this);
}

void Resolver::resolve(const shared_ptr<const Expr> expr)
{
    expr->accept(this);
}

shared_ptr<Object> Resolver::visitVariable(const shared_ptr<const Variable> variable)
{
    if (scopes.size() > 0)
    {
        auto &top = scopes.back();
        auto found = top.find(variable->name.lexeme);
        if (found != top.end() && found->second == false)
        {
            std::cout << variable->name.lexeme << "Can't read local variable in its own initializer." << std::endl;
        }
    }
    resolveLocal(variable, variable->name);
    return nullptr;
}

shared_ptr<Object> Resolver::visitAssign(const shared_ptr<const Assign> assign)
{
    resolve(assign->value);
    resolveLocal(assign, assign->name);
    return nullptr;
}

shared_ptr<Object> Resolver::visitBinary(const shared_ptr<const Binary> binary)
{
    resolve(binary->left);
    resolve(binary->right);
    return nullptr;
}

shared_ptr<Object> Resolver::visitCall(const shared_ptr<const Call> call)
{
    resolve(call->callee);
    for (const auto &arg : call->arguments)
    {
        resolve(arg);
    }
    return nullptr;
}

shared_ptr<Object> Resolver::visitGrouping(const shared_ptr<const Grouping> grouping)
{
    resolve(grouping->expr);
    return nullptr;
}

shared_ptr<Object> Resolver::visitLiteral(const shared_ptr<const Literal> literal)
{
    return nullptr;
}

shared_ptr<Object> Resolver::visitLogical(const shared_ptr<const Logical> logical)
{
    resolve(logical->left);
    resolve(logical->right);
    return nullptr;
}

shared_ptr<Object> Resolver::visitUnary(const shared_ptr<const Unary> unary)
{
    resolve(unary->right);
    return nullptr;
}

shared_ptr<Object> Resolver::visitGet(const shared_ptr<const Get> get)
{
    resolve(get->expr);
    return nullptr;
}

shared_ptr<Object> Resolver::visitSet(const shared_ptr<const Set> set)
{
    resolve(set->value);
    resolve(set->obj);
    return nullptr;
}

shared_ptr<Object> Resolver::visitThis(const shared_ptr<const This> this_)
{
    if (currentClass == ClassType::NONE)
        throw RuntimeError(this_->keyword, "Can't use 'this' outside of a class.");

    resolveLocal(this_, this_->keyword);
    return std::shared_ptr<Object>{};
}

shared_ptr<Object> Resolver::visitSuper(const shared_ptr<const Super> super)
{
    switch (currentClass)
    {
    case ClassType::NONE:
        throw RuntimeError(super->keyword, "Can't use 'super' outside of a class.");
    case ClassType::CLASS:
        throw RuntimeError(super->keyword, "Can't use 'super' in a class with no superclass.");
    default:
        break;
    }
    resolveLocal(super, super->keyword);
    return nullptr;
}

void Resolver::visitExpression(const shared_ptr<const Expression> expr)
{
    resolve(expr->expr);
}

void Resolver::visitIf(const shared_ptr<const If> if_)
{
    resolve(if_->condition);
    resolve(if_->thenBranch);
    if (if_->elseBranch != nullptr)
        resolve(if_->elseBranch);
}

void Resolver::visitBlock(const shared_ptr<const Block> block)
{
    beginScope();
    resolve(block->stmts);
    endScope();
}

void Resolver::visitFunction(const shared_ptr<const Function> func)
{
    declare(func->name);
    define(func->name);
    resolveFunction(func, FunctionType::FUNCTION);
}

void Resolver::visitPrint(const shared_ptr<const Print> print)
{
    resolve(print->expr);
}

void Resolver::visitReturn(const shared_ptr<const Return> return_)
{
    if (currentFunction == FunctionType::NONE)
        throw RuntimeError(return_->token, "Can't return from top-level code.");

    if (return_->value != nullptr)
    {
        if (currentFunction == FunctionType::INITIALIZER)
            throw RuntimeError(return_->token, "Can't return a value from an initializer.");
        resolve(return_->value);
    }
}

void Resolver::visitWhile(const shared_ptr<const While> while_)
{
    resolve(while_->condition);
    loopScopeDepth += 1;
    resolve(while_->body);
    loopScopeDepth -= 1;
}

void Resolver::visitVar(const shared_ptr<const Var> var)
{
    declare(var->token);
    if (var->initializer != nullptr)
        resolve(var->initializer);
    define(var->token);
}

void Resolver::visitBreak(const shared_ptr<const Break> break_)
{
    if (!loopScopeDepth)
        throw RuntimeError(break_->name, "'break' must be inside a loop.");
}

void Resolver::visitClass(const shared_ptr<const Class> klass)
{
    auto enclosingClass = currentClass;
    currentClass = ClassType::CLASS;
    declare(klass->name);
    define(klass->name);

    if (klass->super != nullptr)
    {
        if (klass->name.lexeme == klass->super->name.lexeme)
            throw RuntimeError(klass->super->name, "A class can't inherit from itself.");

        currentClass = ClassType::SUBCLASS;
        resolve(klass->super);
    }

    if (klass->super != nullptr)
    {
        beginScope();
        scopes.back().insert(std::make_pair("super", true));
    }

    beginScope();
    scopes.back().insert(std::make_pair("this", true));
    for (const auto &method : klass->methods)
    {
        auto type =
            method->name.lexeme == "init"
                ? FunctionType::INITIALIZER
                : FunctionType::METHOD;
        resolveFunction(method, type);
    }
    endScope();

    if (klass->super != nullptr)
        currentClass = enclosingClass;
}

void Resolver::resolveLocal(const shared_ptr<const Expr> expr, const Token &name)
{
    for (int i = scopes.size() - 1; i >= 0; i--)
    {
        const auto &scope = scopes[i];
        if (scope.find(name.lexeme) != scope.end())
        {
            auto depth = scopes.size() - 1 - i;
            interpreter.resolve(expr, depth);
            return;
        }
    }
}

void Resolver::resolveFunction(const shared_ptr<const Function> func, FunctionType functionType)
{
    auto enclosingFunction = currentFunction;
    currentFunction = functionType;
    beginScope();
    for (const auto &param : func->params)
    {
        declare(param);
        define(param);
    }
    resolve(func->body);
    endScope();
    currentFunction = enclosingFunction;
}

void Resolver::declare(const Token &name)
{
    if (scopes.size() == 0)
        return;

    auto &scope = scopes.back();
    if (scope.find(name.lexeme) != scope.end())
        throw RuntimeError(name, "Already a variable with this name in the scope.");

    scope[name.lexeme] = false;
}

void Resolver::define(const Token &name)
{
    if (scopes.size() == 0)
        return;

    auto &scope = scopes.back();
    scope[name.lexeme] = true;
}

void Resolver::beginScope()
{
    scopes.emplace_back();
}

void Resolver::endScope()
{
    scopes.pop_back();
}