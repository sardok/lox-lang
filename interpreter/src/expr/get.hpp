#ifndef _GET_HPP_
#define _GET_HPP_
#include <memory>
#include "expr.hpp"
#include "../visitor/visitor.hpp"
#include "../token.hpp"
#include "../object/object.hpp"

class Get : public Expr, public std::enable_shared_from_this<Get>
{
public:
    explicit Get(const std::shared_ptr<Expr> expr, const Token name)
        : expr{expr}, name{std::move(name)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitGet(shared_from_this());
    }

    const std::shared_ptr<Expr> expr;
    const Token name;
};
#endif