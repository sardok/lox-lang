#ifndef _LOX_FUNCTION_HPP_
#define _LOX_FUNCTION_HPP_
#include <string>
#include <vector>
#include <memory>
#include "lox_callable.hpp"
#include "./stmt/function.hpp"
#include "environment.hpp"

class LoxFunction : public LoxCallable
{
public:
    explicit LoxFunction(const std::shared_ptr<const Function> declaration, const std::shared_ptr<Environment> closure, bool isInitializer)
        : declaration{std::move(declaration)}, closure{std::move(closure)}, isInitializer{std::move(isInitializer)}
    {
    }

    std::shared_ptr<Object> call(Interpreter *, std::vector<std::shared_ptr<Object>> arguments) override;
    int arity() const override;
    std::string toString() const override;
    std::shared_ptr<LoxFunction> bind(std::shared_ptr<Object>) const;

private:
    std::shared_ptr<Object> returnThis() const;
    const std::shared_ptr<const Function> declaration;
    const std::shared_ptr<Environment> closure;
    const bool isInitializer;
};
#endif