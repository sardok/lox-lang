#ifndef LITERAL_HPP
#define LITERAL_HPP
#include <string>
#include <iostream>
#include <memory>
#include "expr.hpp"
#include "../object/object.hpp"
#include "../visitor/visitor.hpp"

class Literal : public Expr, public std::enable_shared_from_this<Literal>
{
public:
    explicit Literal(std::shared_ptr<Object> literal)
        : value{std::move(literal)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        return visitor->visitLiteral(shared_from_this());
    }

    const std::shared_ptr<Object> value;
};
#endif