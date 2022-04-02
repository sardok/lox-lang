#ifndef _EXPR_HPP_
#define _EXPR_HPP_
#include "../object/object.hpp"

class Visitor;

class Expr
{
public:
    virtual std::shared_ptr<Object> accept(Visitor *visitor) const = 0;
};
#endif