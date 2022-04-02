#include <iostream>
#include <cstdio>
#include <utility>
#include <memory>
#include "value.hpp"
#include "object.hpp"

using std::move;
using std::shared_ptr;
using std::string;

void printValue(const Value &value)
{
    switch (value.type)
    {
    case ValueType::VAL_BOOL:
    {
        auto val = asBool(move(value));
        printf("%s", val ? "true" : "false");
    }
    break;
    case ValueType::VAL_NUMBER:
        printf("%g", asNumber(move(value)));
        break;
    case ValueType::VAL_NIL:
        printf("nil");
        break;
    case ValueType::VAL_OBJ:
        printObject(value);
        break;
    }
}

string strValue(const Value &value)
{
    switch (value.type)
    {
    case ValueType::VAL_BOOL:
        return asBool(value) ? "true" : "else";
    case ValueType::VAL_NUMBER:
        return std::to_string(asNumber(value));
    case ValueType::VAL_NIL:
        return "nil";
    case ValueType::VAL_OBJ:
        auto obj = asObject(value);
        return strObject(obj);
    }
}