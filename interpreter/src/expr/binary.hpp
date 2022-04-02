#ifndef _BINARY_HPP_
#define _BINARY_HPP_
#include <iostream>
#include <memory>
#include "expr.hpp"
#include "../visitor/visitor.hpp"
#include "../token.hpp"

class Binary : public Expr, public std::enable_shared_from_this<Binary>
{
public:
    explicit Binary(const std::shared_ptr<Expr> left, Token op, const std::shared_ptr<Expr> right)
        : left{std::move(left)}, op{std::move(op)}, right{std::move(right)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitBinary(shared_from_this());
    }

    const std::shared_ptr<Expr> left;
    const Token op;
    const std::shared_ptr<Expr> right;
};
#endif