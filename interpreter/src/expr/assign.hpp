#ifndef _ASSIGN_HPP_
#define _ASSIGN_HPP_
#include <memory>
#include "expr.hpp"
#include "../object/object.hpp"
#include "../visitor/visitor.hpp"
#include "../token.hpp"

class Assign : public Expr, public std::enable_shared_from_this<Assign>
{
public:
    explicit Assign(const Token &name, const std::shared_ptr<Expr> value)
        : name{name}, value{value}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitAssign(shared_from_this());
    }

    const Token name;
    const std::shared_ptr<Expr> value;
};

#endif