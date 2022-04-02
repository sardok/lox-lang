#ifndef _LOX_INSTANCE_HPP_
#define _LOX_INSTANCE_HPP_
#include <string>
#include <map>
#include <memory>
#include "lox_class.hpp"
#include "object/object.hpp"
#include "token.hpp"

class LoxInstance : public Object, public std::enable_shared_from_this<LoxInstance>
{
public:
    explicit LoxInstance(std::shared_ptr<LoxClass> klass);
    std::string toString() const override;
    std::shared_ptr<Object> get(const Token &);
    void set(const Token &, std::shared_ptr<Object>);
    const std::shared_ptr<LoxClass> klass;

private:
    std::map<std::string, std::shared_ptr<Object>> fields;
};
#endif