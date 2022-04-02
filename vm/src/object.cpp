#include <iostream>
#include <utility>
#include <memory>
#include "object.hpp"

using std::shared_ptr;
using std::string;
using std::uint32_t;

uint32_t hashString(const string &str)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < str.size(); i++)
    {
        hash ^= static_cast<uint8_t>(str[i]);
        hash *= 16777619;
    }
    return hash;
}

bool operator==(const StringObject &lhs, const StringObject &rhs)
{
    return lhs.str == rhs.str;
}

string strObject(shared_ptr<Object> obj)
{
    switch (obj->type)
    {
    case ObjectType::OBJECT_STRING:
    {
        auto strObj = static_pointer_cast<StringObject>(obj);
        return strObj->str;
    }
    case ObjectType::OBJECT_NATIVE:
        return "<native>";
    case ObjectType::OBJECT_BOUND_METHOD:
    {
        auto boundMethod = static_pointer_cast<BoundMethodObject>(obj);
        return "<bound " + boundMethod->method->function->name->str + ">";
    }
    case ObjectType::OBJECT_CLASS:
    {
        auto klass = static_pointer_cast<ClassObject>(obj);
        return "<class " + strObject(klass->name) + ">";
    }
    case ObjectType::OBJECT_INSTANCE:
    {
        auto instance = static_pointer_cast<InstanceObject>(obj);
        return "<instance " + strObject(instance->klass->name) + ">";
    }
    case ObjectType::OBJECT_CLOSURE:
    {
        auto closure = static_pointer_cast<ClosureObject>(obj);
        auto fnName =
            closure->function->name
                ? strObject(closure->function->name)
                : "script";
        return "<closure " + fnName + ">";
    }
    case ObjectType::OBJECT_FUNCTION:
    {
        auto funcObj = static_pointer_cast<FunctionObject>(obj);
        return funcObj->name ? "<fn " + funcObj->name->str + ">" : "<script>";
    }
    default:
        return "unknown";
    }
}

void printObject(const Value &value)
{
    assert(value.type == ValueType::VAL_OBJ);
    std::cout << strObject(asObject(value));
}
