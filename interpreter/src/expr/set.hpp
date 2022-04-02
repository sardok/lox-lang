#ifndef _SET_HPP_
#define _SET_HPP_
#include <memory>
#include "expr.hpp"
#include "../token.hpp"
#include "../visitor/visitor.hpp"

class Set : public Expr, public std::enable_shared_from_this<Set>
{
public:
    explicit Set(const std::shared_ptr<Expr> obj, const Token name, const std::shared_ptr<Expr> value)
        : obj{std::move(obj)}, name{std::move(name)}, value{std::move(value)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const
    {
        return visitor->visitSet(shared_from_this());
    }

    const std::shared_ptr<Expr> obj;
    const Token name;
    const std::shared_ptr<Expr> value;
};
#endif