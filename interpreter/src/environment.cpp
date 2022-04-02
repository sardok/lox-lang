#include <string>
#include <map>
#include <memory>
#include <iostream>
#include "environment.hpp"
#include "exception/runtime_error.hpp"

using std::make_shared;
using std::move;
using std::shared_ptr;

Environment::Environment(shared_ptr<Environment> enclosing)
    : env{}, enclosing{move(enclosing)}
{
}

void Environment::define(const Token &token, const shared_ptr<Object> value)
{
    env.insert_or_assign(token.lexeme, value);
}

const shared_ptr<Object> Environment::get(const Token &token) const
{
    if (env.contains(token.lexeme))
        return env.at(token.lexeme);

    if (enclosing)
        return enclosing->get(token);

    throw RuntimeError(token, "Undefined identifier");
}

const shared_ptr<Object> Environment::getAt(int distance, const Token &token) const
{
    auto env = ancestor(distance);
    return env->get(token);
}

void Environment::assign(const Token &token, const shared_ptr<Object> value)
{
    if (env.contains(token.lexeme))
    {
        env[token.lexeme] = value;
        return;
    }

    if (enclosing)
    {
        enclosing->assign(token, value);
        return;
    }

    throw RuntimeError(token, "Undefined variable '" + token.lexeme + "'.");
}

void Environment::assignAt(int depth, const Token &token, const shared_ptr<Object> value)
{
    auto env = const_pointer_cast<Environment>(ancestor(depth));
    env->assign(token, value);
}

std::shared_ptr<const Environment> Environment::ancestor(int distance) const
{
    if (!distance)
        return shared_from_this();

    return enclosing->ancestor(distance - 1);
}
