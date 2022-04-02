#ifndef _CALL_HPP_
#define _CALL_HPP_
#include <memory>
#include <vector>
#include "expr.hpp"
#include "../token.hpp"
#include "../visitor/visitor.hpp"

class Call : public Expr, public std::enable_shared_from_this<Call>
{
public:
    Call(const std::shared_ptr<Expr> callee, const Token token, const std::vector<std::shared_ptr<Expr>> arguments)
        : callee{std::move(callee)}, token{std::move(token)}, arguments{std::move(arguments)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitCall(shared_from_this());
    }

    const std::shared_ptr<Expr> callee;
    const Token token;
    const std::vector<std::shared_ptr<Expr>> arguments;
};
#endif