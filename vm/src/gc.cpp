#include <iostream>
#include <algorithm>
#include "gc.hpp"
#include "vm.hpp"
#include "object.hpp"

using std::move;
using std::shared_ptr;
using std::size_t;
using std::vector;

#define GC_HEAP_GROW_FACTOR 2

Gc::Gc(Vm *vm) : vm{vm}
{
}

void Gc::collectGarbage()
{
#ifdef DEBUG_LOG_GC
    std::cout << "-- gc begin" << std::endl;
    auto before = bytesAllocated;
#endif

    markRoots();
    traceReferences();
    tableRemoveWhite(vm->strings);
    sweep();
    nextGC = bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    std::cout << "-- gc end" << std::endl;
    std::cout << " collected " << before - bytesAllocated << " bytes ";
    std::cout << "(from " << before << " to " << bytesAllocated << ") next at ";
    std::cout << nextGC << std::endl;
#endif
}

void Gc::markRoots()
{
    for (Value *slot = &vm->stack[0]; slot < vm->stackTop; slot++)
    {
        markValue(*slot);
    }

    for (int i = 0; i < vm->frameCount; i++)
    {
        auto &frame = vm->frames[i];
        markObject(frame.closure);
    }

    for (auto upvalue = vm->openUpvalues; upvalue; upvalue = static_pointer_cast<UpvalueObject>(upvalue->next))
    {
        markObject(upvalue);
    }

    markTable(vm->globals);
    markObject(vm->initString);
}

void Gc::markValue(Value &value)
{
    if (isObject(value))
        markObject(asObject(value));
}

void Gc::markObject(shared_ptr<Object> obj)
{
    if (!obj)
        return;
    if (obj->isMarked)
        return;

#ifdef DEBUG_LOG_GC
    std::cout << std::to_address(obj.get()) << " mark ";
    printValue(objectValue(obj));
    std::cout << std::endl;
#endif

    obj->isMarked = true;
    grayStack.push_back(move(obj));
}

void Gc::markTable(Table &table)
{
    for (auto &entry : table.entries)
    {
        markObject(entry.key);
        markValue(entry.value);
    }
}

void Gc::traceReferences()
{
    while (grayStack.size())
    {
        auto obj = grayStack.back();
        grayStack.pop_back();
        blackenObject(move(obj));
    }
}

void Gc::sweep()
{
    shared_ptr<Object> previous{};
    auto object = vm->objects;
    while (object)
    {
        if (object->isMarked)
        {
            object->isMarked = false;
            previous = object;
            object = object->next;
        }
        else
        {
            bytesAllocated -= sizeof(*object);
            object = object->next;
            if (previous)
                previous->next = object;
            else
                vm->objects = object;
        }
    }
}

void Gc::blackenObject(shared_ptr<Object> obj)
{
#ifdef DEBUG_LOG_GC
    std::cout << std::to_address(obj.get()) << " blacken ";
    printValue(objectValue(obj));
    std::cout << std::endl;
#endif

    switch (obj->type)
    {
    case ObjectType::OBJECT_BOUND_METHOD:
    {
        auto boundMethod = static_pointer_cast<BoundMethodObject>(obj);
        markValue(boundMethod->receiver);
        markObject(boundMethod->method);
    }
    break;
    case ObjectType::OBJECT_CLASS:
    {
        auto klass = static_pointer_cast<ClassObject>(obj);
        markObject(klass->name);
        markTable(klass->methods);
    }
    break;
    case ObjectType::OBJECT_INSTANCE:
    {
        auto instance = static_pointer_cast<InstanceObject>(obj);
        markObject(instance->klass);
        markTable(instance->fields);
    }
    break;
    case ObjectType::OBJECT_CLOSURE:
    {
        auto closure = static_pointer_cast<ClosureObject>(obj);
        markObject(closure->function);
        for (auto upvalue : closure->upvalues)
        {
            markObject(upvalue);
        }
    }
    break;
    case ObjectType::OBJECT_FUNCTION:
    {
        auto objFunc = static_pointer_cast<FunctionObject>(obj);
        markObject(objFunc->name);
        markValues(objFunc->chunk.constants);
    }
    break;
    case ObjectType::OBJECT_UPVALUE:
        markValue(static_pointer_cast<UpvalueObject>(obj)->closed);
        break;
    case ObjectType::OBJECT_NATIVE:
    case ObjectType::OBJECT_STRING:
        break;
    default:
        break;
    }
}

void Gc::markValues(vector<Value> &values)
{
    for (auto &value : values)
    {
        markValue(value);
    }
}

void Gc::tableRemoveWhite(Table &table)
{
    while (true)
    {
        auto found = std::find_if(
            table.entries.begin(),
            table.entries.end(),
            [](Entry &entry)
            { return entry.key && !entry.key->isMarked; });

        if (found == table.entries.end())
            break;

        auto res = table.deleteKey((*found).key);
        assert(res == true);
    }
}

void Gc::addToBytesAllocated(size_t bytes)
{
    bytesAllocated += bytes;
}

bool Gc::shouldCollect()
{
    return bytesAllocated > nextGC;
}