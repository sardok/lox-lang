#ifndef _VISITOR_HPP_
#define _VISITOR_HPP_
#include <string>
#include <memory>
#include "../object/object.hpp"

class Binary;
class Grouping;
class Literal;
class Unary;
class Variable;
class Assign;
class Expression;
class Print;
class Var;
class Block;
class If;
class While;
class Logical;
class Break;
class Call;
class Function;
class Return;
class Class;
class Get;
class Set;
class This;
class Super;

using std::shared_ptr;

class Visitor
{
public:
    virtual shared_ptr<Object> visitBinary(const shared_ptr<const Binary>) = 0;
    virtual shared_ptr<Object> visitGrouping(const shared_ptr<const Grouping>) = 0;
    virtual shared_ptr<Object> visitLiteral(const shared_ptr<const Literal>) = 0;
    virtual shared_ptr<Object> visitUnary(const shared_ptr<const Unary>) = 0;
    virtual shared_ptr<Object> visitVariable(const shared_ptr<const Variable>) = 0;
    virtual shared_ptr<Object> visitAssign(const shared_ptr<const Assign>) = 0;
    virtual shared_ptr<Object> visitLogical(const shared_ptr<const Logical>) = 0;
    virtual shared_ptr<Object> visitCall(const shared_ptr<const Call>) = 0;
    virtual shared_ptr<Object> visitGet(const shared_ptr<const Get>) = 0;
    virtual shared_ptr<Object> visitSet(const shared_ptr<const Set>) = 0;
    virtual shared_ptr<Object> visitThis(const shared_ptr<const This>) = 0;
    virtual shared_ptr<Object> visitSuper(const shared_ptr<const Super>) = 0;
    virtual void visitExpression(const shared_ptr<const Expression>) = 0;
    virtual void visitPrint(const shared_ptr<const Print>) = 0;
    virtual void visitVar(const shared_ptr<const Var>) = 0;
    virtual void visitBlock(const shared_ptr<const Block>) = 0;
    virtual void visitIf(const shared_ptr<const If>) = 0;
    virtual void visitWhile(const shared_ptr<const While>) = 0;
    virtual void visitBreak(const shared_ptr<const Break>) = 0;
    virtual void visitFunction(const shared_ptr<const Function>) = 0;
    virtual void visitReturn(const shared_ptr<const Return>) = 0;
    virtual void visitClass(const shared_ptr<const Class>) = 0;
};
#endif