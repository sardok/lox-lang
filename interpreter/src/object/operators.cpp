#include <exception>
#include "object.hpp"
#include "string_object.hpp"
#include "number_object.hpp"

std::ostream &operator<<(std::ostream &os, const StringObject &so)
{
    os << so.ss.str();
    return os;
}

// bool operator==(const Object &lhs, const Object &rhs)
// {
//     auto sol = dynamic_cast<const StringObject *>(&lhs);
//     auto sor = dynamic_cast<const StringObject *>(&rhs);
//     if (sol != nullptr && sor != nullptr)
//         return *sol == *sor;

//     auto nol = dynamic_cast<const NumberObject *>(&lhs);
//     auto nor = dynamic_cast<const NumberObject *>(&rhs);
//     if (nol != nullptr && nor != nullptr)
//         return *nol == *nor;

//     throw std::runtime_error("cannot compare objects");
// }

// bool operator!=(const Object &lhs, const Object &rhs)
// {
//     return !(lhs == rhs);
// }

// bool operator<(const Object &lhs, const Object &rhs)
// {
//     auto sol = dynamic_cast<const StringObject *>(&lhs);
//     auto sor = dynamic_cast<const StringObject *>(&rhs);
//     if (sol != nullptr && sor != nullptr)
//         return *sol < *sor;

//     auto nol = dynamic_cast<const NumberObject *>(&lhs);
//     auto nor = dynamic_cast<const NumberObject *>(&rhs);
//     if (nol != nullptr && nor != nullptr)
//         return *nol < *nor;

//     throw std::runtime_error("cannot compare objects");
// }

bool operator==(const StringObject &lhs, const StringObject &rhs)
{
    return lhs.toString() == rhs.toString();
}

bool operator!=(const StringObject &lhs, const StringObject &rhs)
{
    return !(lhs == rhs);
}

bool operator<(const StringObject &lhs, const StringObject &rhs)
{
    return lhs.toString() < rhs.toString();
}

bool operator>(const StringObject &lhs, const StringObject &rhs)
{
    return rhs < lhs;
}

bool operator<=(const StringObject &lhs, const StringObject &rhs)
{
    return !(lhs > rhs);
}

bool operator>=(const StringObject &lhs, const StringObject &rhs)
{
    return !(lhs < rhs);
}

bool operator==(const NumberObject &lhs, const NumberObject &rhs)
{
    return lhs.number == rhs.number;
}

bool operator!=(const NumberObject &lhs, const NumberObject &rhs)
{
    return !(lhs == rhs);
}

bool operator<(const NumberObject &lhs, const NumberObject &rhs)
{
    return lhs.number < rhs.number;
}

bool operator>(const NumberObject &lhs, const NumberObject &rhs)
{
    return rhs < lhs;
}

bool operator<=(const NumberObject &lhs, const NumberObject &rhs)
{
    return !(lhs > rhs);
}

bool operator>=(const NumberObject &lhs, const NumberObject &rhs)
{
    return !(lhs < rhs);
}