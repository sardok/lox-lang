#ifndef _FUNCTION_HPP_
#define _FUNCTION_HPP_
#include <vector>
#include <memory>
#include "stmt.hpp"
#include "block.hpp"
#include "../token.hpp"
#include "../visitor/visitor.hpp"

class Function : public Stmt, public std::enable_shared_from_this<Function>
{
public:
    explicit Function(const Token name, const std::vector<Token> params, const std::vector<std::shared_ptr<Stmt>> body)
        : name{name}, params{std::move(params)}, body{std::move(body)}
    {
    }

    std::shared_ptr<Object> accept(Visitor *visitor) const override
    {
        visitor->visitFunction(shared_from_this());
        return std::shared_ptr<Object>{};
    }

    const Token name;
    const std::vector<Token> params;
    const std::vector<std::shared_ptr<Stmt>> body;
};
#endif