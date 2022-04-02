#ifndef _WHILE_HPP_
#define _WHILE_HPP_
#include <memory>
#include "stmt.hpp"
#include "../expr/expr.hpp"
#include "../visitor/visitor.hpp"

class While : public Stmt, public std::enable_shared_from_this<While>
{
public:
    explicit While(const std::shared_ptr<Expr> condition, const std::shared_ptr<Stmt> body)
        : condition{std::move(condition)}, body{std::move(body)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitWhile(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const std::shared_ptr<Expr> condition;
    const std::shared_ptr<Stmt> body;
};
#endif