#ifndef _LOGICAL_HPP_
#define _LOGICAL_HPP_
#include <memory>
#include "expr.hpp"
#include "../visitor/visitor.hpp"
#include "../object/object.hpp"

class Logical : public Expr, public std::enable_shared_from_this<Logical>
{
public:
    explicit Logical(const std::shared_ptr<Expr> left, const Token op, const std::shared_ptr<Expr> right)
        : left{std::move(left)}, op{std::move(op)}, right{std::move(right)} {}

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitLogical(shared_from_this());
    }

    const std::shared_ptr<Expr> left;
    const Token op;
    const std::shared_ptr<Expr> right;
};
#endif