#ifndef _RETURN_EXCEPTION_HPP_
#define _RETURN_EXCEPTION_HPP_
#include <exception>
#include <memory>

class Object;

class ReturnException : public std::exception
{
public:
    explicit ReturnException(const std::shared_ptr<Object> value)
        : value{std::move(value)}
    {
    }

    const std::shared_ptr<Object> value;
};
#endif