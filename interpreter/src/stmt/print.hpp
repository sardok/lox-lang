#ifndef _PRINT_HPP_
#define _PRINT_HPP_
#include <memory>
#include "stmt.hpp"
#include "../expr/expr.hpp"
#include "../object/object.hpp"

class Print : public Stmt, public std::enable_shared_from_this<Print>
{
public:
    explicit Print(const std::shared_ptr<Expr> expr) : expr{std::move(expr)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitPrint(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const std::shared_ptr<Expr> expr;
};
#endif