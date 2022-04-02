#ifndef _STMT_HPP_
#define _STMT_HPP_
#include <memory>
#include "../object/object.hpp"

class Visitor;

class Stmt
{
public:
    virtual std::shared_ptr<Object> accept(Visitor *visitor) const = 0;
};
#endif