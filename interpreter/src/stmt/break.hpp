#ifndef _BREAK_HPP_
#define _BREAK_HPP_
#include <memory>
#include "stmt.hpp"
#include "../object/object.hpp"
#include "../visitor/visitor.hpp"
#include "../token.hpp"

class Break : public Stmt, public std::enable_shared_from_this<Break>
{
public:
    explicit Break(const Token name) : name{name} {}

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitBreak(shared_from_this());
        return std::shared_ptr<Object>{};
    }
    const Token name;
};
#endif