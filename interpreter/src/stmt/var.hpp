#ifndef _VAR_HPP_
#define _VAR_HPP_
#include <memory>
#include "stmt.hpp"
#include "../expr/expr.hpp"
#include "../token.hpp"

class Var : public Stmt, public std::enable_shared_from_this<Var>
{
public:
    explicit Var(const Token token, const std::shared_ptr<Expr> initializer)
        : token{std::move(token)}, initializer{std::move(initializer)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitVar(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const Token token;
    const std::shared_ptr<Expr> initializer;
};
#endif