#ifndef _NUMBER_OBJECT_HPP_
#define _NUMBER_OBJECT_HPP_
#include <iostream>
#include "object.hpp"
class NumberObject : public Object
{
public:
    explicit NumberObject(double number) : number{number}
    {
    }

    explicit NumberObject(std::string number) : number{std::stod(number)}
    {
    }

    std::string toString() const override
    {
        auto str = std::to_string(number);
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        if (str.back() == '.')
            str.pop_back();
        return str;
    }

    friend bool operator<(const NumberObject &lhs, const NumberObject &rhs);
    friend bool operator>(const NumberObject &lhs, const NumberObject &rhs);
    friend bool operator<=(const NumberObject &lhs, const NumberObject &rhs);
    friend bool operator>=(const NumberObject &lhs, const NumberObject &rhs);
    friend bool operator==(const NumberObject &lhs, const NumberObject &rhs);
    friend bool operator!=(const NumberObject &lhs, const NumberObject &rhs);

    const double number;
};
#endif