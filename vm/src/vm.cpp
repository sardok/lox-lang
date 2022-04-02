#include <exception>
#include <iostream>
#include <cstdarg>
#include <utility>
#include "vm.hpp"
#include "disassembler.hpp"
#include "compiler.hpp"
#include "object.hpp"
#include "native.hpp"

using std::move;
using std::optional;
using std::shared_ptr;
using std::uint16_t;
using std::uint8_t;

#define BINARY_OP(valueType, op)                             \
    do                                                       \
    {                                                        \
        if (!isNumber(peek(0)) || !isNumber(peek(1)))        \
        {                                                    \
            runtimeError("Operands must be number.");        \
            return InterpretResult::INTERPRET_RUNTIME_ERROR; \
        }                                                    \
        auto b = asNumber(pop());                            \
        auto a = asNumber(pop());                            \
        push(valueType(a op b));                             \
    } while (false)

Vm::Vm() : stackTop{&stack[0]}, gc{this}
{
    defineNative("clock", clockNative);
    initString = makeString("init");
}

InterpretResult Vm::interpret(std::string &source)
{
    shared_ptr<FunctionObject> funcObj;

    {
        auto compiler = createCompiler();
        auto [result, funcObjOpt] = compiler.compile(source);
        if (result == CompileResult::COMPILE_ERROR)
            return InterpretResult::INTERPRET_COMPILE_ERROR;

        funcObj = funcObjOpt.value();
    }

    pushObject(funcObj);
    auto closure = createAndAddObject(newClosure, funcObj);
    pop();
    push(objectValue(closure));
    call(move(closure), 0);
    return run();
}

Compiler Vm::createCompiler()
{
    TryFindInternedStringFunc tryFindInternedString = [this](std::string &key)
    { return this->findString(key); };

    AddStringToInternFunc addStringToIntern = [this](shared_ptr<StringObject> obj)
    { this->addString(obj); };

    auto stringInternProps = StringInternProps{tryFindInternedString, addStringToIntern};
    return Compiler{stringInternProps};
}

void Vm::setChunk(Chunk *chunk)
{
    this->chunk = chunk;
    this->code = chunk->getCode();
    this->ip = &this->code[0];
}

InterpretResult Vm::run()
{
    auto frame = &frames[frameCount - 1];

    auto readByte = [&frame]()
    { return *frame->ip++; };
    auto readShort = [&frame]()
    {
        frame->ip += 2;
        return static_cast<uint16_t>((frame->ip[-2] << 8) | frame->ip[-1]);
    };
    auto readConstant = [&frame, &readByte]()
    {
        return frame->closure->function->chunk.getConstant(readByte());
    };
    auto readString = [&readConstant]()
    { return asString(readConstant()); };

#ifdef DEBUG_TRACE_EXECUTION
    auto disasm = Disassembler{&frame->closure->function->chunk};
#endif
    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (auto slot = stack.begin(); slot < stackTop; slot++)
        {
            std::cout << "[ ";
            printValue(*slot);
            std::cout << "] ";
        }
        std::cout << std::endl;
        disasm.disassembleInstruction(frame->ip - frame->closure->function->chunk.getCodeBaseAddr());
#endif

        auto instruction = static_cast<Opcode>(readByte());
        switch (instruction)
        {
        case Opcode::OP_CONSTANT:
            push(readConstant());
            break;
        case Opcode::OP_NIL:
            push(NilVal);
            break;
        case Opcode::OP_TRUE:
            push(TrueVal);
            break;
        case Opcode::OP_FALSE:
            push(FalseVal);
            break;
        case Opcode::OP_POP:
            pop();
            break;
        case Opcode::OP_GET_LOCAL:
        {
            auto slot = readByte();
            push(frame->slots[slot]);
        }
        break;
        case Opcode::OP_SET_LOCAL:
        {
            auto slot = readByte();
            frame->slots[slot] = peek(0);
        }
        break;
        case Opcode::OP_GET_GLOBAL:
        {
            auto name = readString();
            // std::string s = name && name->str.size() > 0 ? name->str : "unknown";
            // std::cout << "getting global from " << name << std::endl;
            auto res = globals.get(name);
            if (!res)
            {
                runtimeError("Undefined variable '%s'", name->str.c_str());
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }
            push(res.value());
        }
        break;
        case Opcode::OP_DEFINE_GLOBAL:
        {
            auto name = readString();
            globals.set(name, peek(0));
            pop();
        }
        break;
        case Opcode::OP_SET_GLOBAL:
        {
            auto name = readString();
            if (globals.set(name, peek(0)))
            {
                globals.deleteKey(name);
                runtimeError("Undefined variable '%s'", name->str.c_str());
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }
        }
        break;
        case Opcode::OP_GET_UPVALUE:
        {
            auto slot = readByte();
            push(*frame->closure->upvalues[slot]->location);
        }
        break;
        case Opcode::OP_SET_UPVALUE:
        {
            auto slot = readByte();
            *frame->closure->upvalues[slot]->location = peek(0);
        }
        break;
        case Opcode::OP_GET_PROPERTY:
        {
            if (!isInstance(peek(0)))
            {
                runtimeError("Only instances have properties.");
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }

            auto instance = asInstance(peek(0));
            auto name = readString();
            auto res = instance->fields.get(name);
            if (res)
            {
                pop(); // instance
                push(res.value());
                break;
            }

            if (!bindMethod(instance->klass, name))
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        break;
        case Opcode::OP_SET_PROPERTY:
        {
            if (!isInstance(peek(1)))
            {
                runtimeError("Only instances have fields.");
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }

            auto instance = asInstance(peek(1));
            instance->fields.set(readString(), peek(0));
            auto value = pop();
            pop(); // instance
            push(value);
        }
        break;
        case Opcode::OP_GET_SUPER:
        {
            auto name = readString();
            auto superclass = asClass(pop());

            if (!bindMethod(superclass, name))
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        break;
        case Opcode::OP_EQUAL:
        {
            auto b = pop();
            auto a = pop();
            push(boolValue(valuesEqual(a, b)));
        }
        break;
        case Opcode::OP_GREATER:
            BINARY_OP(boolValue, >);
            break;
        case Opcode::OP_LESS:
            BINARY_OP(boolValue, <);
            break;
        case Opcode::OP_ADD:
            if (isString(peek(0)) && isString(peek(1)))
            {
                concatenate();
            }
            else if (isNumber(peek(0)) && isNumber(peek(1)))
            {
                BINARY_OP(numberValue, +);
            }
            else if (isString(peek(0)) || isString(peek(1)))
            {
                auto b = pop();
                auto a = pop();
                pushObject(makeString(strValue(a) + strValue(b)));
            }
            else
            {
                runtimeError("Operands mismatch.");
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }
            break;
        case Opcode::OP_SUBTRACT:
            BINARY_OP(numberValue, -);
            break;
        case Opcode::OP_DIVIDE:
            BINARY_OP(numberValue, /);
            break;
        case Opcode::OP_MULTIPLY:
            BINARY_OP(numberValue, *);
            break;
        case Opcode::OP_NOT:
            push(boolValue(isFalsey(pop())));
            break;
        case Opcode::OP_NEGATE:
            if (!isNumber(peek(0)))
            {
                runtimeError("Operand must be a number.");
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }
            push(numberValue(-asNumber(pop())));
            break;
        case Opcode::OP_PRINT:
            printValue(pop());
            std::cout << std::endl;
            break;
        case Opcode::OP_JUMP:
        {
            auto offset = readShort();
            frame->ip += offset;
        }
        break;
        case Opcode::OP_JUMP_IF_FALSE:
        {
            auto offset = readShort();
            if (isFalsey(peek(0)))
                frame->ip += offset;
        }
        break;
        case Opcode::OP_LOOP:
        {
            auto offset = readShort();
            frame->ip -= offset;
        }
        break;
        case Opcode::OP_CALL:
        {
            auto argCount = readByte();
            if (!callValue(peek(argCount), argCount))
                return InterpretResult::INTERPRET_RUNTIME_ERROR;

            frame = &frames[frameCount - 1];
        }
        break;
        case Opcode::OP_INVOKE:
        {
            auto method = readString();
            auto argCount = readByte();

            if (!invoke(method, argCount))
                return InterpretResult::INTERPRET_RUNTIME_ERROR;

            frame = &frames[frameCount - 1];
        }
        break;
        case Opcode::OP_INVOKE_SUPER:
        {
            auto method = readString();
            auto argCount = readByte();
            auto superclass = asClass(pop());

            if (!invokeFromClass(superclass, method, argCount))
                return InterpretResult::INTERPRET_RUNTIME_ERROR;

            frame = &frames[frameCount - 1];
        }
        break;
        case Opcode::OP_CLOSURE:
        {
            auto closure = createAndAddObject(newClosure, asFunction(readConstant()));
            push(objectValue(closure));
            for (int i = 0; i < closure->upvalueCount; i++)
            {
                auto isLocal = readByte();
                auto index = readByte();
                if (isLocal)
                {
                    closure->upvalues[i] = captureUpvalue(frame->slots + index);
                }
                else
                {
                    closure->upvalues[i] = frame->closure->upvalues[i];
                }
            }
        }
        break;
        case Opcode::OP_CLOSE_UPVALUE:
            closeUpvalues(stackTop - 1);
            pop();
            break;
        case Opcode::OP_RETURN:
        {
            auto result = pop();
            closeUpvalues(frame->slots);
            frameCount--;
            if (frameCount == 0)
            {
                pop();
                gc.collectGarbage();
                return InterpretResult::INTERPRET_OK;
            }

            stackTop = frame->slots;
            push(result);
            frame = &frames[frameCount - 1];
        }
        break;
        case Opcode::OP_INHERIT:
        {
            auto superclass = peek(1);
            if (!isClass(superclass))
            {
                runtimeError("Superclass must be a class.");
                return InterpretResult::INTERPRET_RUNTIME_ERROR;
            }
            auto subclass = asClass(peek(0));
            subclass->methods.addAll(asClass(superclass)->methods);
            pop(); // subclass
        }
        break;
        case Opcode::OP_CLASS:
        {
            auto klass = createAndAddObject(newClass, asString(readConstant()));
            push(objectValue(klass));
        }
        break;
        case Opcode::OP_METHOD:
            defineMethod(readString());
            break;
        default:
            break;
        }
    }
    return InterpretResult::INTERPRET_RUNTIME_ERROR;
}

void Vm::push(Value val)
{
    *stackTop = move(val);
    stackTop++;
}

void Vm::pushObject(shared_ptr<Object> obj)
{
    push(objectValue(move(obj)));
}

Value Vm::pop()
{
    if (stackTop == stack.begin())
        throw std::runtime_error("Empty stack.");

    stackTop--;
    return *stackTop;
}

Value Vm::peek(int distance)
{
    return *(stackTop - 1 - distance);
}

bool Vm::call(shared_ptr<ClosureObject> closure, int argCount)
{
    if (closure->function->arity != argCount)
    {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (frameCount == FRAMES_MAX)
    {
        runtimeError("Stack overflow.");
        return false;
    }

    auto frame = &frames[frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.getCodeBaseAddr();
    frame->slots = stackTop - argCount - 1;
    return true;
}

bool Vm::callValue(Value callee, int argCount)
{
    if (isObject(callee))
    {
        switch (asObject(callee)->type)
        {
        case ObjectType::OBJECT_BOUND_METHOD:
        {
            auto bound = asBoundMethod(callee);
            stackTop[-argCount - 1] = bound->receiver;
            return call(bound->method, argCount);
        }
        case ObjectType::OBJECT_CLASS:
        {
            auto klass = asClass(callee);
            stackTop[-argCount - 1] = objectValue(newInstance(klass));
            auto init = klass->methods.get(initString);
            if (init)
                return call(asClosure(init.value()), argCount);
            else if (argCount > 0)
            {
                runtimeError("Expected 0 arguments, but got '%d'.", argCount);
                return false;
            }
            return true;
        }
        case ObjectType::OBJECT_CLOSURE:
            return call(asClosure(callee), argCount);
        case ObjectType::OBJECT_NATIVE:
        {
            auto nativeFunction = asNative(callee);
            auto result = nativeFunction->function(argCount, stackTop - argCount);
            stackTop -= (argCount + 1);
            push(result);
            return true;
        }
        default:
            break;
        }
    }

    runtimeError("Can only call functions and classes.");
    return false;
}

bool Vm::invokeFromClass(shared_ptr<ClassObject> &klass, shared_ptr<StringObject> &name, int argCount)
{
    auto method = klass->methods.get(name);
    if (!method)
    {
        runtimeError("Undefined property '%s'.", name->str.c_str());
        return false;
    }

    return call(asClosure(method.value()), argCount);
}

bool Vm::invoke(shared_ptr<StringObject> &name, int argCount)
{
    auto receiver = peek(argCount);
    if (!isInstance(receiver))
    {
        runtimeError("Only instances have methods.");
        return false;
    }
    auto instance = asInstance(receiver);

    auto field = instance->fields.get(name);
    if (field)
    {
        stackTop[-argCount - 1] = field.value();
        return callValue(field.value(), argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

bool Vm::bindMethod(shared_ptr<ClassObject> &klass, shared_ptr<StringObject> &name)
{
    auto res = klass->methods.get(name);
    if (!res)
    {
        runtimeError("Undefined property '%s'.", name->str.c_str());
        return false;
    }

    auto boundMethod = createAndAddObject(newBoundMethod, peek(0), asClosure(res.value()));
    pop();
    push(objectValue(boundMethod));
    return true;
}

shared_ptr<UpvalueObject> Vm::captureUpvalue(Value *local)
{
    shared_ptr<UpvalueObject> prevUpvalue{};
    auto upvalue = dynamic_pointer_cast<UpvalueObject>(openUpvalues);
    while (upvalue && upvalue->location > local)
    {
        prevUpvalue = upvalue;
        upvalue = dynamic_pointer_cast<UpvalueObject>(upvalue->next);
    }

    if (upvalue && upvalue->location == local)
        return upvalue;

    auto createdUpvalue = createAndAddObject(newUpvalue, local);
    createdUpvalue->next = upvalue;
    if (prevUpvalue)
        prevUpvalue->next = createdUpvalue;
    else
        openUpvalues = createdUpvalue;

    return createdUpvalue;
}

void Vm::closeUpvalues(Value *last)
{
    while (openUpvalues && openUpvalues->location >= last)
    {
        auto upvalue = openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        openUpvalues = dynamic_pointer_cast<UpvalueObject>(upvalue->next);
    }
}

void Vm::defineMethod(shared_ptr<StringObject> name)
{
    auto method = peek(0);
    auto klass = asClass(peek(1));
    klass->methods.set(move(name), move(method));
    pop();
}

void Vm::runtimeError(const char *format, ...)
{
    std::va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = frameCount - 1; i >= 0; i--)
    {
        auto &frame = frames[i];
        auto &chunk = frame.closure->function->chunk;
        auto instruction = frame.ip - chunk.getCodeBaseAddr() - 1;
        int line = chunk.getLine(instruction);
        auto name =
            frame.closure->function->name
                ? frame.closure->function->name->str.c_str()
                : "script";

        fprintf(stderr, "[line %d] in %s\n", line, name);
    }
    resetStack();
}

void Vm::defineNative(std::string name, NativeFn function)
{
    push(objectValue(makeString(move(name))));
    push(objectValue(createAndAddObject(newNative, function)));
    globals.set(asString(peek(1)), peek(0));
    pop();
    pop();
}

void Vm::resetStack()
{
    stackTop = &stack[0];
    frameCount = 0;
}

bool Vm::isFalsey(Value val)
{
    return isNil(val) || (isBool(val) && !asBool(val));
}

void Vm::concatenate()
{
    auto b = asString(peek(0));
    auto a = asString(peek(1));
    auto s = makeString(a->str + b->str);
    pop();
    pop();
    pushObject(move(s));
}

bool Vm::valuesEqual(Value &val1, Value &val2)
{
    if (val1.type != val2.type)
        return false;

    switch (val1.type)
    {
    case ValueType::VAL_BOOL:
        return asBool(val1) == asBool(val2);
    case ValueType::VAL_NIL:
        return true;
    case ValueType::VAL_NUMBER:
        return asNumber(val1) == asNumber(val2);
    case ValueType::VAL_OBJ:
        return asObject(val1) == asObject(val2);
    }
}

shared_ptr<StringObject> Vm::makeString(std::string str)
{
    auto found = findString(str);
    if (found)
        return found.value();

    auto obj = createAndAddObject(newString, move(str));
    addString(obj);
    return obj;
}

optional<shared_ptr<StringObject>> Vm::findString(std::string &str)
{
    return strings.findKey(str, hashString(str));
}

void Vm::addString(shared_ptr<StringObject> obj)
{
    strings.set(move(obj), NilVal);
}

template <ConceptObject T, typename... Args>
shared_ptr<T> Vm::createAndAddObject(shared_ptr<T> (*factory)(Args...), Args... args)
{
    auto obj = factory(std::forward<Args>(args)...);
    addObject(obj);

#ifdef DEBUG_LOG_GC
    // std::cout << std::to_address(obj.get()) << " allocate for " << static_cast<int>(obj->type) << std::endl;
#endif
    collectGarbageIfNeeded<T>();
    return obj;
}

template <ConceptObject T>
shared_ptr<T> Vm::createAndAddObject(shared_ptr<T> (*factory)())
{
    auto obj = factory();
    addObject(obj);

#ifdef DEBUG_LOG_GC
    // std::cout << std::to_address(obj.get()) << " allocate for " << static_cast<int>(obj->type) << std::endl;
#endif
    collectGarbageIfNeeded<T>();
    return obj;
}

template <ConceptObject T>
void Vm::collectGarbageIfNeeded()
{
    gc.addToBytesAllocated(sizeof(T));
    if (gc.shouldCollect())
        gc.collectGarbage();
}

void Vm::addObject(shared_ptr<Object> obj)
{
    obj->next = objects;
    objects = obj;
}

void Vm::freeObjects()
{
    objects = shared_ptr<Object>();
}