#ifndef _CLOCK_HPP_
#define _CLOCK_HPP_
#include "../lox_callable.hpp"
#include "../object/object.hpp"
#include "../visitor/interpreter.hpp"

class Clock : public LoxCallable
{
public:
    explicit Clock() = default;
    std::shared_ptr<Object> call(Interpreter *, std::vector<std::shared_ptr<Object>>) override;
    int arity() const override;
    std::string toString() const override;
};
#endif