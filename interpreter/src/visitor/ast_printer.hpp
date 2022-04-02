#ifndef _AST_PRINTER_VISITOR_HPP_
#define _AST_PRINTER_VISITOR_HPP_
#include <array>
#include <span>
#include <sstream>
#include <vector>
#include <sstream>
#include <memory>
#include "visitor.hpp"
#include "../expr/binary.hpp"
#include "../expr/grouping.hpp"
#include "../expr/literal.hpp"
#include "../expr/unary.hpp"
#include "../expr/variable.hpp"
#include "../expr/assign.hpp"
#include "../expr/logical.hpp"
#include "../expr/call.hpp"
#include "../expr/get.hpp"
#include "../expr/set.hpp"
#include "../expr/this.hpp"
#include "../object/string_object.hpp"
#include "../stmt/expression.hpp"
#include "../stmt/print.hpp"
#include "../stmt/var.hpp"
#include "../stmt/block.hpp"
#include "../stmt/if.hpp"
#include "../stmt/while.hpp"
#include "../stmt/break.hpp"
#include "../stmt/function.hpp"
#include "../stmt/return.hpp"
#include "../stmt/class.hpp"

class AstPrinter : public Visitor
{
public:
    std::string print(const std::vector<Stmt *> stmts)
    {
        std::stringstream ss;
        for (const auto &stmt : stmts)
        {
            ss << print(stmt);
        }

        return ss.str();
    }

    std::string print(const std::shared_ptr<Expr> expr)
    {
        auto so = dynamic_pointer_cast<StringObject>(expr->accept(this));
        return so->toString();
    }

    std::string print(const Stmt *stmt)
    {
        stmt->accept(this);
        return "nil";
    }

    std::shared_ptr<Object> visitBinary(const std::shared_ptr<const Binary> binary) override
    {
        std::shared_ptr<Expr> params[]{binary->left, binary->right};
        auto s = parenthesize(binary->op.lexeme, params);
        return std::make_shared<StringObject>(s);
    }

    std::shared_ptr<Object> visitGrouping(const std::shared_ptr<const Grouping> grouping) override
    {
        std::shared_ptr<Expr> params[]{grouping->expr};
        auto s = parenthesize("group", params);
        return std::make_shared<StringObject>(s);
    }

    std::shared_ptr<Object> visitLiteral(const std::shared_ptr<const Literal> literal) override
    {
        auto s = literal->value->toString();
        return std::make_shared<StringObject>(s);
    }

    std::shared_ptr<Object> visitUnary(const std::shared_ptr<const Unary> unary) override
    {
        std::shared_ptr<Expr> params[]{unary->right};
        auto s = parenthesize(unary->op.lexeme, params);
        return std::make_shared<StringObject>(s);
    }

    std::shared_ptr<Object> visitVariable(const std::shared_ptr<const Variable> var) override
    {
        return std::make_shared<StringObject>("var " + var->name.lexeme);
    }

    std::shared_ptr<Object> visitAssign(const std::shared_ptr<const Assign> assign) override
    {
        return std::make_shared<StringObject>(assign->name.lexeme + "=" + assign->value->accept(this)->toString());
    }

    std::shared_ptr<Object> visitLogical(const std::shared_ptr<const Logical> logical) override
    {
        auto left = dynamic_pointer_cast<StringObject>(logical->left->accept(this));
        auto right = dynamic_pointer_cast<StringObject>(logical->right->accept(this));
        return std::make_shared<StringObject>(left->toString() + logical->op.lexeme + right->toString());
    }

    std::shared_ptr<Object> visitCall(const std::shared_ptr<const Call> call) override
    {
        auto left = dynamic_pointer_cast<StringObject>(call->callee->accept(this));
        std::stringstream ss;
        for (auto argument : call->arguments)
        {
            ss << argument->accept(this)->toString();
            ss << ", ";
        }
        ss.seekp(-2, std::ios_base::end);
        return std::make_shared<StringObject>(left->toString() + "(" + ss.str() + ")");
    }

    std::shared_ptr<Object> visitGet(const std::shared_ptr<const Get> get) override
    {
        auto expr = get->expr->accept(this);
        return std::make_shared<StringObject>("Get " + get->name.lexeme + " from " + expr->toString());
    }

    std::shared_ptr<Object> visitSet(const std::shared_ptr<const Set> set) override
    {
        auto obj = set->obj->accept(this);
        auto value = set->obj->accept(this);
        return std::make_shared<StringObject>("Set " + obj->toString() + " to " + value->toString());
    }

    std::shared_ptr<Object> visitThis(const std::shared_ptr<const This> this_) override
    {
        return std::make_shared<StringObject>(this_->keyword.lexeme);
    }

    void visitExpression(const std::shared_ptr<const Expression> expr) override
    {
        std::cout << "Visiting expression stmt" << std::endl;
        // expr->expr->accept(this);
    }

    void visitPrint(const std::shared_ptr<const Print> print) override
    {
        std::cout << "Visiting print stmt" << std::endl;
        // print->expr->accept(this);
    }

    void visitVar(const std::shared_ptr<const Var> var) override
    {
        std::cout << "Visiting var stmt" << std::endl;
        // var->initializer->accept(this);
    }

    void visitBlock(const std::shared_ptr<const Block> block) override
    {

        std::cout << "Visiting block stmt" << std::endl;
        // for (const auto &stmt : block->stmts)
        //     stmt->accept(this);
    }

    void visitIf(const std::shared_ptr<const If> if_) override
    {
        std::cout << "Visiting if stmt" << std::endl;
        // if_->condition->accept(this);
        // if_->thenBranch->accept(this);
        // if (if_->elseBranch != nullptr)
        //     if_->elseBranch->accept(this);
    }

    void visitWhile(const std::shared_ptr<const While> while_) override
    {
        std::cout << "Visiting while stmt" << std::endl;
        // while_->condition->accept(this);
        // while_->body->accept(this);
    }

    void visitBreak(const std::shared_ptr<const Break> break_) override
    {
        std::cout << "Visiting break stmt" << std::endl;
    }

    void visitFunction(const std::shared_ptr<const Function> fun) override
    {
        std::cout << "Visiting function stmt" << std::endl;
        // fun->body->accept(this);
    }

    void visitReturn(const std::shared_ptr<const Return> ret) override
    {
        std::cout << "Visiting return stmt" << std::endl;
        // if (ret->value != nullptr)
        //     ret->value->accept(this);
    }

    void visitClass(const std::shared_ptr<const Class> klass) override
    {
        std::cout << "Visiting class stmt" << std::endl;
        // for (const auto &method : klass->methods)
        //     method->accept(this);
    }

private:
    std::string parenthesize(const std::string &name, const std::span<std::shared_ptr<Expr>> &exprs)
    {
        std::stringstream ss{};
        ss << "(" << name;
        for (const auto &expr : exprs)
        {
            ss << " ";
            auto so = dynamic_pointer_cast<StringObject>(expr->accept(this));
            ss << *so;
        }
        ss << ")";
        return ss.str();
    }
};
#endif