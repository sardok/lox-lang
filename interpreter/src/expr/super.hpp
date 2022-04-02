#ifndef _SUPER_HPP_
#define _SUPER_HPP_
#include <memory>
#include "expr.hpp"
#include "../visitor/visitor.hpp"
#include "../token.hpp"

class Super : public Expr, public std::enable_shared_from_this<Super>
{
public:
    explicit Super(const Token keyword, const Token method)
        : keyword{std::move(keyword)}, method{std::move(method)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitSuper(shared_from_this());
    }

    const Token keyword;
    const Token method;
};
#endif