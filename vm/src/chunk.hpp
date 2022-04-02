#ifndef _CHUNK_HPP_
#define _CHUNK_HPP_
#include <vector>
#include <span>
#include "value.hpp"
#include "gc.hpp"

enum class Opcode : std::uint8_t
{
    OP_CONSTANT,
    OP_CONSTANT_32,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_INVOKE_SUPER,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD,
};

class Chunk
{
public:
    explicit Chunk(){};
    void write(Opcode, int);
    void write(std::uint8_t, int);
    void write(std::uint32_t, int);
    std::span<const std::uint8_t> getCode() const;
    int addConstant(Value);
    const std::uint8_t operator[](std::size_t) const;
    std::uint8_t &operator[](std::size_t);
    std::size_t size() const;
    Value getConstant(int index) const;
    int getLine(int index) const;
    std::uint8_t *getCodeBaseAddr();

private:
    friend Gc;
    std::vector<std::uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;
};
#endif