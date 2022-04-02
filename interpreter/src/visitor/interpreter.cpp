#include <string>
#include <variant>
#include <utility>
#include <span>
#include <vector>
#include <memory>
#include "interpreter.hpp"
#include "../object/object.hpp"
#include "../object/number_object.hpp"
#include "../object/string_object.hpp"
#include "../object/bool_object.hpp"
#include "../object/nil_object.hpp"
#include "../expr/binary.hpp"
#include "../expr/grouping.hpp"
#include "../expr/literal.hpp"
#include "../expr/unary.hpp"
#include "../expr/variable.hpp"
#include "../expr/assign.hpp"
#include "../expr/logical.hpp"
#include "../expr/call.hpp"
#include "../expr/get.hpp"
#include "../expr/set.hpp"
#include "../expr/this.hpp"
#include "../expr/super.hpp"
#include "../stmt/expression.hpp"
#include "../stmt/print.hpp"
#include "../stmt/var.hpp"
#include "../stmt/block.hpp"
#include "../stmt/if.hpp"
#include "../stmt/while.hpp"
#include "../stmt/break.hpp"
#include "../stmt/function.hpp"
#include "../stmt/return.hpp"
#include "../stmt/class.hpp"
#include "../token.hpp"
#include "../token_type.hpp"
#include "../exception/runtime_error.hpp"
#include "../exception/return_exception.hpp"
#include "../lox_callable.hpp"
#include "../lox_function.hpp"
#include "../lox_class.hpp"
#include "../lox_instance.hpp"
#include "../globals/clock.hpp"

using std::make_shared;
using std::move;
using std::shared_ptr;
using std::vector;

class BreakExecutionException : public std::exception
{
};

Interpreter::Interpreter()
{
    globals = std::make_shared<Environment>();
    environment = globals;
    registerGlobals();
}

void Interpreter::interpret(const vector<shared_ptr<Stmt>> &stmts)
{
    for (const auto &stmt : stmts)
    {
        interpret(stmt);
    }
}

void Interpreter::interpret(const shared_ptr<Stmt> &stmt)
{
    stmt->accept(this);
}

void Interpreter::resolve(const shared_ptr<const Expr> expr, int depth)
{
    locals[expr] = depth;
}

shared_ptr<Object> evaluateBinary(const shared_ptr<NumberObject> &lhs, const shared_ptr<NumberObject> &rhs, const Token &op)
{
    switch (op.type)
    {
    case TokenType::MINUS:
        return make_shared<NumberObject>(lhs->number - rhs->number);
    case TokenType::PLUS:
        return make_shared<NumberObject>(lhs->number + rhs->number);
    case TokenType::SLASH:
        return make_shared<NumberObject>(lhs->number / rhs->number);
    case TokenType::STAR:
        return make_shared<NumberObject>(lhs->number * rhs->number);
    case TokenType::GREATER:
        return make_shared<BoolObject>(*lhs > *rhs);
    case TokenType::GREATER_EQUAL:
        return make_shared<BoolObject>(*lhs >= *rhs);
    case TokenType::LESSER:
        return make_shared<BoolObject>(*lhs < *rhs);
    case TokenType::LESSER_EQUAL:
        return make_shared<BoolObject>(*lhs <= *rhs);
    case TokenType::EQUAL_EQUAL:
        return make_shared<BoolObject>(*lhs == *rhs);
    case TokenType::BANG_EQUAL:
        return make_shared<BoolObject>(*lhs != *rhs);
    default:
        break;
    }
    throw std::runtime_error("Unsupported operator for number: " + op.lexeme);
}

shared_ptr<Object> evaluateBinary(const shared_ptr<StringObject> &lhs, const shared_ptr<StringObject> &rhs, const Token &op)
{
    auto cmpRes = false;
    switch (op.type)
    {
    case TokenType::PLUS:
        return make_shared<StringObject>(lhs->toString() + rhs->toString());
    case TokenType::GREATER:
        cmpRes = *lhs > *rhs;
        break;
    case TokenType::GREATER_EQUAL:
        cmpRes = *lhs > *rhs;
        break;
    case TokenType::LESSER:
        cmpRes = *lhs < *rhs;
        break;
    case TokenType::LESSER_EQUAL:
        cmpRes = *lhs <= *rhs;
        break;
    case TokenType::EQUAL_EQUAL:
        cmpRes = *lhs == *rhs;
        break;
    case TokenType::BANG_EQUAL:
        cmpRes = *lhs != *rhs;
        break;
    default:
        throw std::runtime_error("Unsupported operator for string: " + op.lexeme);
    }

    return make_shared<BoolObject>(cmpRes);
}

shared_ptr<Object> Interpreter::visitBinary(const shared_ptr<const Binary> binary)
{
    auto left = evaluate(binary->left);
    auto right = evaluate(binary->right);

    auto nol = dynamic_pointer_cast<NumberObject>(left);
    auto nor = dynamic_pointer_cast<NumberObject>(right);
    if (nol && nor)
        return evaluateBinary(move(nol), move(nor), binary->op);

    auto sol = dynamic_pointer_cast<StringObject>(left);
    auto sor = dynamic_pointer_cast<StringObject>(right);
    if (sol && sor)
        return evaluateBinary(move(sol), move(sor), binary->op);

    auto sol1 = dynamic_pointer_cast<StringObject>(left);
    if (sol1)
        return evaluateBinary(move(sol1), make_shared<StringObject>(right->toString()), binary->op);

    auto sor1 = dynamic_pointer_cast<StringObject>(right);
    if (sor1)
        return evaluateBinary(make_shared<StringObject>(left->toString()), move(sor1), binary->op);

    switch (binary->op.type)
    {
    case TokenType::EQUAL_EQUAL:
        return make_shared<BoolObject>(isEqual(left, right));
    case TokenType::BANG_EQUAL:
        return make_shared<BoolObject>(!isEqual(left, right));
    default:
        break;
    }

    throwOperandsMismatch(binary->op, left, right);
    return nullptr;
}

shared_ptr<Object> Interpreter::visitGrouping(const shared_ptr<const Grouping> grouping)
{
    return evaluate(grouping->expr);
}

shared_ptr<Object> Interpreter::visitLiteral(const shared_ptr<const Literal> literal)
{
    return literal->value;
}

shared_ptr<Object> Interpreter::visitUnary(const shared_ptr<const Unary> unary)
{
    auto right = evaluate(unary->right);

    switch (unary->op.type)
    {
    case TokenType::BANG:
    {
        auto value = !isTruthy(right);
        return make_shared<BoolObject>(value);
    }
    case TokenType::MINUS:
    {
        auto no = dynamic_pointer_cast<NumberObject>(right);
        if (no)
            return make_shared<NumberObject>(-no->number);
    }
    default:
        break;
    }

    throw RuntimeError(unary->op, "Unsupported unary");
}

shared_ptr<Object> Interpreter::visitVariable(const shared_ptr<const Variable> variable)
{
    return lookupVariable(variable->name, variable);
}

shared_ptr<Object> Interpreter::visitAssign(const shared_ptr<const Assign> assign)
{
    auto value = evaluate(assign->value);
    auto entry = locals.find(assign);
    if (entry != locals.end())
    {
        auto distance = entry->second;
        environment->assignAt(distance, assign->name, std::as_const(value));
    }
    else
    {
        globals->assign(assign->name, std::as_const(value));
    }
    return value;
}

shared_ptr<Object> Interpreter::visitLogical(const shared_ptr<const Logical> logical)
{
    auto left = evaluate(logical->left);

    if (logical->op.type == TokenType::OR)
    {
        if (isTruthy(left))
            return left;
    }
    else
    {
        if (!isTruthy(left))
            return left;
    }

    return evaluate(logical->right);
}

shared_ptr<Object> Interpreter::visitCall(const shared_ptr<const Call> call)
{
    auto callee = evaluate(call->callee);
    vector<shared_ptr<Object>> arguments;
    for (const auto argument : call->arguments)
    {
        arguments.push_back(evaluate(argument));
    }

    auto callable = dynamic_pointer_cast<LoxCallable>(callee);
    if (callable)
    {
        throwIfArgumentsMismatch(call->token, callable, arguments);
        return callable->call(this, move(arguments));
    }

    throw RuntimeError(call->token, "Invalid callable.");
}

shared_ptr<Object> Interpreter::visitGet(const shared_ptr<const Get> get)
{
    auto obj = evaluate(get->expr);
    auto loxInstance = dynamic_pointer_cast<LoxInstance>(obj);
    if (loxInstance)
        return loxInstance->get(get->name);

    throw RuntimeError(get->name, "Only instances have properties.");
}

shared_ptr<Object> Interpreter::visitSet(const shared_ptr<const Set> set)
{
    auto obj = evaluate(set->obj);
    auto loxInstance = dynamic_pointer_cast<LoxInstance>(obj);
    if (!loxInstance)
        throw RuntimeError(set->name, "Only instances have fields.");

    auto value = evaluate(set->value);
    loxInstance->set(set->name, value);
    return value;
}

shared_ptr<Object> Interpreter::visitThis(const shared_ptr<const This> this_)
{
    return lookupVariable(this_->keyword, this_);
}

shared_ptr<Object> Interpreter::visitSuper(const shared_ptr<const Super> super)
{
    auto local = locals.find(super);
    if (local == locals.end())
        throw RuntimeError(super->keyword, "Unable to find distance to super.");
    auto distance = local->second;

    auto superClass = dynamic_pointer_cast<LoxClass>(environment->getAt(distance, super->keyword));
    if (!superClass)
        throw RuntimeError(super->keyword, "Unable to find super class.");

    auto thisInstance = dynamic_pointer_cast<LoxInstance>(
        environment->getAt(distance - 1, Token{TokenType::THIS, "this", std::nullopt, -1}));
    if (!thisInstance)
        throw RuntimeError(super->keyword, "Unable to find 'this' instance for super.");

    auto method = superClass->findMethod(super->method.lexeme);
    if (!method.has_value())
        throw RuntimeError(super->method, "Undefined properpty '" + super->method.lexeme + "'.");

    return method.value()->bind(thisInstance);
}

void Interpreter::visitExpression(const shared_ptr<const Expression> expr)
{
    evaluate(expr->expr);
}

void Interpreter::visitPrint(const shared_ptr<const Print> print)
{
    auto res = evaluate(print->expr);
    std::cout << res->toString() << std::endl;
}

void Interpreter::visitVar(const shared_ptr<const Var> var)
{
    auto value =
        var->initializer != nullptr
            ? evaluate(var->initializer)
            : make_shared<NilObject>();
    environment->define(var->token, std::as_const(value));
}

void Interpreter::visitBlock(const shared_ptr<const Block> block)
{
    auto env = std::make_shared<Environment>(environment);
    executeBlock(block->stmts, std::move(env));
}

void Interpreter::visitIf(const shared_ptr<const If> if_)
{
    if (isTruthy(evaluate(if_->condition)))
    {
        interpret(if_->thenBranch);
    }
    else if (if_->elseBranch != nullptr)
    {
        interpret(if_->elseBranch);
    }
}

void Interpreter::visitWhile(const shared_ptr<const While> while_)
{
    while (isTruthy(evaluate(while_->condition)))
    {
        try
        {
            interpret(while_->body);
        }
        catch (const BreakExecutionException &)
        {
            break;
        }
    }
}

void Interpreter::visitBreak(const shared_ptr<const Break> break_)
{
    throw BreakExecutionException{};
}

void Interpreter::visitFunction(const shared_ptr<const Function> function)
{
    auto closure = environment;
    auto loxFunction = make_shared<LoxFunction>(function, environment, false);
    environment->define(function->name, loxFunction);
}

void Interpreter::visitReturn(const shared_ptr<const Return> return_)
{
    auto value =
        return_->value != nullptr
            ? evaluate(return_->value)
            : make_shared<NilObject>();

    throw ReturnException{value};
}

void Interpreter::visitClass(const shared_ptr<const Class> klass)
{
    std::shared_ptr<LoxClass> superClass;
    if (klass->super != nullptr)
    {
        auto loxClass = dynamic_pointer_cast<LoxClass>(evaluate(klass->super));
        if (!loxClass)
            throw RuntimeError(klass->super->name, "Superclass must be a class.");
        superClass = loxClass;
    }

    environment->define(klass->name, nullptr);

    if (superClass)
    {
        environment = std::make_shared<Environment>(environment);
        Token super{TokenType::SUPER, "super", std::nullopt, -1};
        environment->define(super, superClass);
    }

    std::map<std::string, std::shared_ptr<LoxFunction>> methods;
    for (auto method : klass->methods)
    {
        auto function = std::make_shared<LoxFunction>(method, environment, method->name.lexeme == "init");
        methods[method->name.lexeme] = std::move(function);
    }
    auto loxClass = make_shared<LoxClass>(klass->name, superClass, std::move(methods));

    if (superClass)
        environment = environment->enclosing;

    environment->assign(klass->name, loxClass);
}

void Interpreter::executeBlock(const std::vector<shared_ptr<Stmt>> &body, std::shared_ptr<Environment> executionEnv)
{
    auto previous = environment;
    environment = executionEnv;

    auto cleanup = [&]
    {
        environment = previous;
    };
    try
    {
        interpret(body);
        cleanup();
    }
    catch (const std::exception &)
    {
        cleanup();
        throw;
    }
}

void Interpreter::registerGlobals()
{
    globals->define(Token{TokenType::IDENTIFIER, "clock", std::nullopt, -1}, make_shared<Clock>());
}

shared_ptr<Object> Interpreter::evaluate(const shared_ptr<Expr> expr)
{
    return expr->accept(this);
}

bool Interpreter::isTruthy(const shared_ptr<Object> &obj) const
{
    if (!obj)
        return false;

    auto bo = dynamic_pointer_cast<BoolObject>(obj);
    if (bo)
        return bo->value;

    return true;
}

bool Interpreter::isEqual(const shared_ptr<Object> &lhs, const shared_ptr<Object> &rhs) const
{
    if (!lhs && !rhs)
        return true;
    return false;
}

void Interpreter::throwOperandsMismatch(const Token &token, const shared_ptr<Object> &lhs, const shared_ptr<Object> &rhs) const noexcept(false)
{
    std::stringstream ss;
    ss << "Operand types mismatch " << lhs->toString();
    ss << ", " << rhs->toString();
    throw RuntimeError(token, ss.str());
}

void Interpreter::throwIfArgumentsMismatch(const Token &token, const shared_ptr<LoxCallable> &callable, std::span<shared_ptr<Object>> arguments) const noexcept(false)
{
    if (callable->arity() != arguments.size())
    {
        std::stringstream ss;
        ss << "Expected " << callable->arity() << " arguments but got ";
        ss << arguments.size() << ".";
        throw RuntimeError(token, ss.str());
    }
}

shared_ptr<Object> Interpreter::lookupVariable(const Token &name, const shared_ptr<const Expr> expr)
{
    auto found = locals.find(expr);
    auto obj =
        found != locals.end()
            ? environment->getAt(found->second, name)
            : globals->get(name);
    return obj;
}