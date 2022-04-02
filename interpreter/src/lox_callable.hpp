#ifndef _LOX_CALLABLE_HPP_
#define _LOX_CALLABLE_HPP_
#include <vector>
#include <string>
#include <memory>
#include "object/object.hpp"

class Interpreter;

class LoxCallable : public Object
{
public:
    virtual std::shared_ptr<Object> call(Interpreter *, std::vector<std::shared_ptr<Object>> arguments) = 0;
    virtual int arity() const = 0;
    virtual std::string toString() const = 0;
};
#endif