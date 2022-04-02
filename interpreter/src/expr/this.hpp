#ifndef _THIS_HPP_
#define _THIS_HPP_
#include <memory>
#include "expr.hpp"
#include "../token.hpp"
#include "../visitor/visitor.hpp"

class This : public Expr, public std::enable_shared_from_this<This>
{
public:
    explicit This(const Token keyword) : keyword{std::move(keyword)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitThis(shared_from_this());
    }

    const Token keyword;
};
#endif