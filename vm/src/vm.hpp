#ifndef _VM_HPP_
#define _VM_HPP_
#include <span>
#include <array>
#include <string>
#include <memory>
#include <concepts>
#include <type_traits>
#include "chunk.hpp"
#include "value.hpp"
#include "object.hpp"
#include "table.hpp"
#include "compiler.hpp"
#include "gc.hpp"

enum class InterpretResult
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

template <typename T>
concept ConceptObject = std::is_base_of<Object, T>::value;

struct CallFrame
{
    explicit CallFrame() = default;

    std::shared_ptr<ClosureObject> closure{};
    std::uint8_t *ip = nullptr;
    Value *slots = nullptr;
};

class Vm
{
public:
    explicit Vm();
    InterpretResult interpret(std::string &);

private:
    friend Gc;
    Compiler createCompiler();
    void setChunk(Chunk *);
    InterpretResult run();
    void push(Value);
    void pushObject(std::shared_ptr<Object>);
    Value pop();
    Value peek(int);
    bool call(std::shared_ptr<ClosureObject>, int);
    bool callValue(Value, int);
    bool invokeFromClass(std::shared_ptr<ClassObject> &, std::shared_ptr<StringObject> &, int);
    bool invoke(std::shared_ptr<StringObject> &, int);
    bool bindMethod(std::shared_ptr<ClassObject> &, std::shared_ptr<StringObject> &);
    std::shared_ptr<UpvalueObject> captureUpvalue(Value *);
    void closeUpvalues(Value *);
    void defineMethod(std::shared_ptr<StringObject>);
    void runtimeError(const char *, ...);
    void defineNative(std::string, NativeFn);
    void resetStack();
    bool isFalsey(Value);
    bool valuesEqual(Value &, Value &);
    void concatenate();
    std::shared_ptr<StringObject> makeString(std::string);
    std::optional<std::shared_ptr<StringObject>> findString(std::string &);
    void addString(std::shared_ptr<StringObject>);
    template <ConceptObject T, typename... Args>
    std::shared_ptr<T> createAndAddObject(std::shared_ptr<T> (*)(Args...), Args...);
    template <ConceptObject T>
    std::shared_ptr<T> createAndAddObject(std::shared_ptr<T> (*)());
    template <ConceptObject T>
    void collectGarbageIfNeeded();
    void addObject(std::shared_ptr<Object>);
    void freeObjects();
    const std::uint8_t *ip = nullptr;
    Chunk *chunk = nullptr;
    std::span<const uint8_t> code;
    std::array<Value, STACK_MAX> stack;
    Value *stackTop = nullptr;
    std::shared_ptr<Object> objects{};
    Table globals;
    Table strings;
    std::array<CallFrame, FRAMES_MAX> frames;
    int frameCount = 0;
    std::shared_ptr<UpvalueObject> openUpvalues{};
    Gc gc;
    std::shared_ptr<StringObject> initString{};
};
#endif