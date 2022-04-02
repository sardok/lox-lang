#ifndef _LOX_CLASS_HPP_
#define _LOX_CLASS_HPP_
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include "./object/object.hpp"
#include "lox_callable.hpp"
#include "token.hpp"

class Interpreter;
class LoxFunction;

class LoxClass : public LoxCallable, public std::enable_shared_from_this<LoxClass>
{
public:
    explicit LoxClass(const Token token, const std::shared_ptr<LoxClass> super, const std::map<std::string, std::shared_ptr<LoxFunction>>);
    std::shared_ptr<Object> call(Interpreter *, std::vector<std::shared_ptr<Object>> args) override;
    int arity() const override;
    std::string toString() const override;
    std::optional<std::shared_ptr<LoxFunction>> findMethod(std::string name) const;
    const std::shared_ptr<LoxClass> super;
    const Token name;

private:
    const std::map<std::string, std::shared_ptr<LoxFunction>> methods;
};
#endif