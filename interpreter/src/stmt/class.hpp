#ifndef _CLASS_HPP_
#define _CLASS_HPP_
#include <vector>
#include <memory>
#include "stmt.hpp"
#include "../token.hpp"
#include "../visitor/visitor.hpp"

class Function;
class Variable;

class Class : public Stmt, public std::enable_shared_from_this<Class>
{
public:
    Class(const Token name, const std::shared_ptr<Variable> super, std::vector<shared_ptr<Function>> methods)
        : name{std::move(name)}, super{super}, methods{std::move(methods)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitClass(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const Token name;
    const std::shared_ptr<Variable> super;
    const std::vector<std::shared_ptr<Function>> methods;
};
#endif