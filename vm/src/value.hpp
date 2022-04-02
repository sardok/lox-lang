#ifndef _VALUE_HPP_
#define _VALUE_HPP_
#include <variant>
#include <memory>
enum class ValueType
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
};

class Object;

struct Value
{
    ValueType type;
    std::variant<bool, double, std::shared_ptr<Object>> as;
};

void printValue(const Value &);
std::string strValue(const Value &);

inline Value boolValue(bool b)
{
    return Value{ValueType::VAL_BOOL, b};
}

inline Value numberValue(double num)
{
    return Value{ValueType::VAL_NUMBER, std::move(num)};
}

inline Value objectValue(std::shared_ptr<Object> obj)
{
    return Value{ValueType::VAL_OBJ, std::move(obj)};
}

inline bool asBool(const Value value)
{
    return std::get<bool>(value.as);
}

inline double asNumber(const Value value)
{
    return std::get<double>(value.as);
}

inline std::shared_ptr<Object> asObject(const Value &value)
{
    return std::get<std::shared_ptr<Object>>(value.as);
}

inline bool isBool(const Value value)
{
    return value.type == ValueType::VAL_BOOL;
}

inline bool isNumber(const Value value)
{
    return value.type == ValueType::VAL_NUMBER;
}

inline bool isNil(const Value value)
{
    return value.type == ValueType::VAL_NIL;
}

inline bool isObject(const Value &value)
{
    return value.type == ValueType::VAL_OBJ;
}

const Value TrueVal = {ValueType::VAL_BOOL, true};
const Value FalseVal = {ValueType::VAL_BOOL, false};
const Value NilVal = {ValueType::VAL_NIL, false};
#endif