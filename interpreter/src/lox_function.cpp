#include <vector>
#include <memory>
#include "lox_function.hpp"
#include "environment.hpp"
#include "./object/object.hpp"
#include "./visitor/interpreter.hpp"
#include "./exception/return_exception.hpp"

using std::make_shared;
using std::shared_ptr;
using std::vector;

shared_ptr<Object> LoxFunction::call(Interpreter *interpreter, vector<shared_ptr<Object>> arguments)
{
    auto env = std::make_shared<Environment>(closure);
    for (auto i = 0; i < arguments.size(); i++)
    {
        auto &param = declaration->params[i];
        auto argument = arguments[i];
        env->define(param, argument);
    }

    try
    {
        interpreter->executeBlock(declaration->body, std::move(env));
    }
    catch (ReturnException &returnFromFunction)
    {
        if (isInitializer)
            return returnThis();

        return returnFromFunction.value;
    }

    if (isInitializer)
        return returnThis();

    return nullptr;
}

int LoxFunction::arity() const
{
    return declaration->params.size();
}

std::string LoxFunction::toString() const
{
    return "<fn " + declaration->name.lexeme + ">";
}

shared_ptr<Object> LoxFunction::returnThis() const
{
    Token token{TokenType::THIS, "this", std::nullopt, -1};
    return closure->getAt(0, token);
}

shared_ptr<LoxFunction> LoxFunction::bind(shared_ptr<Object> instance) const
{
    auto env = std::make_shared<Environment>(closure);
    auto token = Token{TokenType::THIS, "this", std::nullopt, -1};
    env->define(token, instance);
    return make_shared<LoxFunction>(declaration, std::move(env), isInitializer);
}