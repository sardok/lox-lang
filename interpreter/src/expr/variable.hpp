#ifndef _VARIABLE_HPP_
#define _VARIABLE_HPP_
#include <memory>
#include "expr.hpp"
#include "../token.hpp"

class Variable : public Expr, public std::enable_shared_from_this<Variable>
{
public:
    explicit Variable(const Token name) : name{name}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitVariable(shared_from_this());
    }

    const Token name;
};
#endif