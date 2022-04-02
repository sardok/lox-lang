#ifndef _UNARY_HPP_
#define _UNARY_HPP_
#include <iostream>
#include "expr.hpp"
#include "../token.hpp"
#include "../visitor/visitor.hpp"

class Unary : public Expr, public std::enable_shared_from_this<Unary>
{
public:
    explicit Unary(const Token op, const shared_ptr<Expr> right)
        : op{op}, right{right}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitUnary(shared_from_this());
    }

    const Token op;
    const shared_ptr<Expr> right;
};
#endif