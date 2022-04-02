#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_
#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <iostream>
#include "value.hpp"
#include "chunk.hpp"
#include "common.hpp"
#include "table.hpp"

std::uint32_t hashString(const std::string &);

enum class ObjectType
{
    OBJECT_BOUND_METHOD,
    OBJECT_CLASS,
    OBJECT_CLOSURE,
    OBJECT_FUNCTION,
    OBJECT_INSTANCE,
    OBJECT_NATIVE,
    OBJECT_STRING,
    OBJECT_UPVALUE,
};

using NativeFn = Value (*)(int argCount, Value *args);

struct Object
{
    ObjectType type;
    bool isMarked = false;
    virtual ~Object() = default;
    std::shared_ptr<Object> next{};

protected:
    Object(ObjectType type) : type{std::move(type)} {}
};

struct StringObject : public Object
{
    explicit StringObject() : Object{ObjectType::OBJECT_STRING}, str{}, hash{} {}
    explicit StringObject(std::string str, std::uint32_t hash)
        : str{std::move(str)}, Object{ObjectType::OBJECT_STRING}, hash{hash} {}

    std::string str;
    std::uint32_t hash;
};

struct UpvalueObject : public Object
{
    explicit UpvalueObject(Value *location)
        : Object{ObjectType::OBJECT_UPVALUE}, location{location} {}

    Value *location;
    Value closed{NilVal};
};

struct Upvalue
{
    std::uint8_t index;
    bool isLocal;
};

struct FunctionObject : public Object
{
    explicit FunctionObject(int arity, std::shared_ptr<StringObject> name)
        : Object{ObjectType::OBJECT_FUNCTION}, arity{std::move(arity)}, chunk{}, name{std::move(name)} {}

    explicit FunctionObject()
        : Object{ObjectType::OBJECT_FUNCTION} {}

    int arity = 0;
    int upvalueCount = 0;
    Chunk chunk;
    std::shared_ptr<StringObject> name{};
};

struct ClosureObject : public Object
{
    // explicit ClosureObject(std::shared_ptr<FunctionObject> function)
    //     : Object{ObjectType::OBJECT_CLOSURE}, function{std::move(function)}, upvalueCount{function->upvalueCount}, upvalues(new std::shared_ptr<UpvalueObject>[function->upvalueCount])
    // {
    //     std::cout << "initialized closure object for upvalue count " << function->upvalueCount << std::endl;
    // }

    explicit ClosureObject(std::shared_ptr<FunctionObject> function)
        : Object{ObjectType::OBJECT_CLOSURE}, function{function}, upvalueCount{function->upvalueCount}
    {
        upvalues.resize(function->upvalueCount);
    }

    std::shared_ptr<FunctionObject> function;
    // std::unique_ptr<std::shared_ptr<UpvalueObject>[]> upvalues;
    std::vector<std::shared_ptr<UpvalueObject>> upvalues;
    int upvalueCount;
};

struct ClassObject : public Object
{
    explicit ClassObject(std::shared_ptr<StringObject> name)
        : Object{ObjectType::OBJECT_CLASS}, name{std::move(name)} {}

    std::shared_ptr<StringObject> name{};
    Table methods;
};

struct InstanceObject : public Object
{
    explicit InstanceObject(std::shared_ptr<ClassObject> klass)
        : Object{ObjectType::OBJECT_INSTANCE}, klass{std::move(klass)} {}

    std::shared_ptr<ClassObject> klass{};
    Table fields;
};

struct BoundMethodObject : public Object
{
    explicit BoundMethodObject(Value receiver, std::shared_ptr<ClosureObject> method)
        : Object{ObjectType::OBJECT_BOUND_METHOD}, receiver{receiver}, method{method} {}

    Value receiver;
    std::shared_ptr<ClosureObject> method;
};

struct NativeObject : public Object
{
    explicit NativeObject(NativeFn function)
        : Object{ObjectType::OBJECT_NATIVE}, function{std::move(function)} {}

    NativeFn function;
};

bool operator==(const StringObject &, const StringObject &);

inline ObjectType objectType(const Value value)
{
    return asObject(value)->type;
}

inline bool isObjectType(const Value value, ObjectType type)
{
    return isObject(value) && objectType(value) == type;
}

inline bool isString(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_STRING);
}

inline bool isFunction(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_FUNCTION);
}

inline bool isClosure(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_CLOSURE);
}

inline bool isClass(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_CLASS);
}

inline bool isInstance(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_INSTANCE);
}

inline bool isBoundMethod(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_BOUND_METHOD);
}

inline bool isNative(const Value value)
{
    return isObjectType(value, ObjectType::OBJECT_NATIVE);
}

inline std::shared_ptr<StringObject> asString(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_STRING);
    return std::static_pointer_cast<StringObject>(std::move(obj));
}

inline std::shared_ptr<FunctionObject> asFunction(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_FUNCTION);
    return std::static_pointer_cast<FunctionObject>(std::move(obj));
}

inline std::shared_ptr<ClosureObject> asClosure(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_CLOSURE);
    return std::static_pointer_cast<ClosureObject>(std::move(obj));
}

inline std::shared_ptr<ClassObject> asClass(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_CLASS);
    return std::static_pointer_cast<ClassObject>(std::move(obj));
}

inline std::shared_ptr<InstanceObject> asInstance(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_INSTANCE);
    return std::static_pointer_cast<InstanceObject>(std::move(obj));
}

inline std::shared_ptr<BoundMethodObject> asBoundMethod(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_BOUND_METHOD);
    return std::static_pointer_cast<BoundMethodObject>(std::move(obj));
}

inline std::shared_ptr<NativeObject> asNative(const Value value)
{
    auto obj = asObject(value);
    assert(obj->type == ObjectType::OBJECT_NATIVE);
    return std::static_pointer_cast<NativeObject>(std::move(obj));
}

inline std::shared_ptr<StringObject> newString(std::string str)
{
    auto hash = hashString(str);
    return std::make_shared<StringObject>(std::move(str), std::move(hash));
}

inline std::shared_ptr<UpvalueObject> newUpvalue(Value *slot)
{
    return std::make_shared<UpvalueObject>(slot);
}

inline std::shared_ptr<FunctionObject> newFunction()
{
    return std::make_shared<FunctionObject>();
}

inline std::shared_ptr<ClosureObject> newClosure(std::shared_ptr<FunctionObject> function)
{
    return std::make_shared<ClosureObject>(std::move(function));
}

inline std::shared_ptr<ClassObject> newClass(std::shared_ptr<StringObject> name)
{
    return std::make_shared<ClassObject>(std::move(name));
}

inline std::shared_ptr<InstanceObject> newInstance(std::shared_ptr<ClassObject> klass)
{
    return std::make_shared<InstanceObject>(std::move(klass));
}

inline std::shared_ptr<BoundMethodObject> newBoundMethod(Value value, std::shared_ptr<ClosureObject> method)
{
    return std::make_shared<BoundMethodObject>(std::move(value), std::move(method));
}

inline std::shared_ptr<NativeObject> newNative(NativeFn fn)
{
    return std::make_shared<NativeObject>(std::move(fn));
}

inline std::shared_ptr<Value> newVal()
{
    auto val = new Value{ValueType::VAL_BOOL, false};
    return std::shared_ptr<Value>{val};
}

std::string strObject(std::shared_ptr<Object>);
void printObject(const Value &value);
#endif