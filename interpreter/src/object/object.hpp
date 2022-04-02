#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_
#include <string>

class Object
{
public:
    virtual std::string toString() const = 0;
};
#endif