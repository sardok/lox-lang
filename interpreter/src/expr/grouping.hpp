#ifndef _GROUPING_HPP_
#define _GROUPING_HPP_
#include <iostream>
#include <memory>
#include "expr.hpp"
#include "../visitor/visitor.hpp"

class Grouping : public Expr, public std::enable_shared_from_this<Grouping>
{
public:
    explicit Grouping(std::shared_ptr<Expr> expr)
        : expr{std::move(expr)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitGrouping(shared_from_this());
    }

    std::shared_ptr<Expr> expr;
};
#endif