#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_
#include <vector>
#include <memory>
#include "stmt.hpp"
#include "../visitor/visitor.hpp"

class Block : public Stmt, public std::enable_shared_from_this<Block>
{
public:
    Block(const std::vector<std::shared_ptr<Stmt>> stmts) : stmts{std::move(stmts)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitBlock(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const std::vector<std::shared_ptr<Stmt>> stmts;
};
#endif