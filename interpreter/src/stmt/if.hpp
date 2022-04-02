#ifndef _IF_HPP_
#define _IF_HPP_
#include <memory>
#include "stmt.hpp"
#include "../expr/expr.hpp"
#include "../visitor/visitor.hpp"
#include "../object/object.hpp"

class If : public Stmt, public std::enable_shared_from_this<If>
{
public:
    explicit If(const std::shared_ptr<Expr> condition, const std::shared_ptr<Stmt> thenBranch, const std::shared_ptr<Stmt> elseBranch)
        : condition{std::move(condition)}, thenBranch{std::move(thenBranch)}, elseBranch{std::move(elseBranch)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitIf(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const std::shared_ptr<Expr> condition;
    const std::shared_ptr<Stmt> thenBranch;
    const std::shared_ptr<Stmt> elseBranch;
};
#endif