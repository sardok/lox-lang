#ifndef _EXPRESSION_HPP_
#define _EXPRESSION_HPP_
#include <memory>
#include "stmt.hpp"
#include "../expr/expr.hpp"
#include "../object/object.hpp"

class Expression : public Stmt, public std::enable_shared_from_this<Expression>
{
public:
    explicit Expression(std::shared_ptr<Expr> expr) : expr{std::move(expr)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitExpression(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const std::shared_ptr<Expr> expr;
};
#endif