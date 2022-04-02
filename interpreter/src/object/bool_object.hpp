#ifndef _BOOL_OBJECT_HPP_
#define _BOOL_OBJECT_HPP_
#include "object.hpp"
class BoolObject : public Object
{
public:
    explicit BoolObject(bool value) : value{value}
    {
    }

    std::string toString() const override
    {
        return value ? "true" : "false";
    }
    const bool value;
};
#endif