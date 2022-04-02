#ifndef _ENVIRONMENT_HPP_
#define _ENVIRONMENT_HPP_
#include <string>
#include <map>
#include <memory>
#include "object/object.hpp"
#include "token.hpp"

class Environment : public std::enable_shared_from_this<Environment>
{
public:
    explicit Environment() = default;
    explicit Environment(std::shared_ptr<Environment>);
    void define(const Token &, const std::shared_ptr<Object> obj);
    const std::shared_ptr<Object> get(const Token &) const;
    const std::shared_ptr<Object> getAt(int, const Token &) const;
    void assign(const Token &, const std::shared_ptr<Object>);
    void assignAt(int, const Token &, const std::shared_ptr<Object>);
    std::shared_ptr<Environment> enclosing;

private:
    std::shared_ptr<const Environment> ancestor(int) const;
    std::map<std::string, std::shared_ptr<Object>> env;
};
#endif