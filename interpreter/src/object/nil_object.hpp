#ifndef _NIL_OBJECT_HPP_
#define _NIL_OBJECT_HPP_
#include "object.hpp"

class NilObject : public Object
{
public:
    std::string toString() const override
    {
        return "nil";
    }
};
#endif