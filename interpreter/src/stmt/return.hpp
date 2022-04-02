#ifndef _RETURN_HPP_
#define _RETURN_HPP_
#include <memory>
#include "stmt.hpp"
#include "../token.hpp"
#include "../expr/expr.hpp"

class Return : public Stmt, public std::enable_shared_from_this<Return>
{
public:
    explicit Return(const Token token, const std::shared_ptr<Expr> value)
        : token{std::move(token)}, value{std::move(value)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitReturn(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const Token token;
    const std::shared_ptr<Expr> value;
};
#endif