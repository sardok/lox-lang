#ifndef _STRING_OBJECT_HPP_
#define _STRING_OBJECT_HPP_
#include <sstream>
#include "object.hpp"

class StringObject : public Object
{
public:
    explicit StringObject() : ss{}
    {
    }

    explicit StringObject(std::string s) : ss{s}
    {
    }

    friend std::ostream &operator<<(std::ostream &os, const StringObject &so);

    std::string toString() const override
    {
        return ss.str();
    }

    friend bool operator<(const StringObject &lhs, const StringObject &rhs);
    friend bool operator>(const StringObject &lhs, const StringObject &rhs);
    friend bool operator<=(const StringObject &lhs, const StringObject &rhs);
    friend bool operator>=(const StringObject &lhs, const StringObject &rhs);
    friend bool operator==(const StringObject &lhs, const StringObject &rhs);
    friend bool operator!=(const StringObject &lhs, const StringObject &rhs);

private:
    std::stringstream ss;
};
#endif