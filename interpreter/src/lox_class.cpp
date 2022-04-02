#include <iostream>
#include "lox_class.hpp"
#include "lox_instance.hpp"
#include "token.hpp"
#include "object/object.hpp"
#include "visitor/interpreter.hpp"
#include "lox_function.hpp"

using std::make_shared;
using std::map;
using std::shared_ptr;
using std::vector;

LoxClass::LoxClass(const Token token, const shared_ptr<LoxClass> super, const std::map<std::string, shared_ptr<LoxFunction>> methods)
    : name{std::move(token)}, super{super}, methods{std::move(methods)}
{
}

shared_ptr<Object> LoxClass::call(Interpreter *interpreter, vector<shared_ptr<Object>> args)
{
    auto instance = make_shared<LoxInstance>(shared_from_this());
    auto method = findMethod("init");
    if (method.has_value())
    {
        auto init = method.value();
        init->bind(instance)->call(interpreter, args);
    }

    return instance;
}

int LoxClass::arity() const
{
    auto method = findMethod("init");
    if (method.has_value())
        return method.value()->arity();

    return 0;
}

std::string LoxClass::toString() const
{
    return name.lexeme;
}

std::optional<std::shared_ptr<LoxFunction>> LoxClass::findMethod(std::string name) const
{
    auto method = methods.find(name);
    if (method != methods.end())
        return std::make_optional(method->second);

    if (super)
        return super->findMethod(name);

    return std::nullopt;
}