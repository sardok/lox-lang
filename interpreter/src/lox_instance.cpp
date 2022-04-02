#include "lox_class.hpp"
#include "lox_instance.hpp"
#include "lox_function.hpp"
#include "./exception/runtime_error.hpp"

using std::move;
using std::shared_ptr;

LoxInstance::LoxInstance(shared_ptr<LoxClass> klass) : klass{move(klass)}
{
}

std::string LoxInstance::toString() const
{
    return klass->name.lexeme + " instance";
}

shared_ptr<Object> LoxInstance::get(const Token &name)
{
    auto field = fields.find(name.lexeme);
    if (field != fields.end())
        return field->second;

    auto method = klass->findMethod(name.lexeme);
    if (method.has_value())
    {
        auto func = method.value();
        return func->bind(shared_from_this());
    }

    throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token &name, shared_ptr<Object> value)
{
    fields.insert_or_assign(name.lexeme, move(value));
}