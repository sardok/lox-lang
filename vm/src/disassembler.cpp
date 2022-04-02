#include <iostream>
#include <cstdio>
#include "disassembler.hpp"
#include "object.hpp"

using std::move;
using std::string;
using std::uint16_t;
using std::uint8_t;

void Disassembler::disassembleChunk(string name)
{
    std::cout << "== " << name << " ==" << std::endl;

    for (auto offset = 0; offset < chunk->size();)
    {
        offset = disassembleInstruction(offset);
    }
}

int Disassembler::disassembleInstruction(int offset)
{
    printf("%04d ", static_cast<int>(offset));
    if (offset > 0 && chunk->getLine(offset) == chunk->getLine(offset - 1))
        std::cout << "   | ";
    else
        printf("%4d ", chunk->getLine(offset));

    auto instruction = static_cast<Opcode>((*chunk)[offset]);
    switch (static_cast<Opcode>(instruction))
    {
    case Opcode::OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", offset);
    case Opcode::OP_CONSTANT_32:
        return constantLongInstruction("OP_CONSTANT_32", offset);
    case Opcode::OP_NIL:
        return simpleInstruction("OP_NIL", offset);
    case Opcode::OP_TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case Opcode::OP_FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case Opcode::OP_POP:
        return simpleInstruction("OP_POP", offset);
    case Opcode::OP_GET_LOCAL:
        return byteInstruction("OP_GET_LOCAL", offset);
    case Opcode::OP_SET_LOCAL:
        return byteInstruction("OP_SET_LOCAL", offset);
    case Opcode::OP_GET_GLOBAL:
        return constantInstruction("OP_GET_GLOBAL", offset);
    case Opcode::OP_DEFINE_GLOBAL:
        return constantInstruction("OP_DEFINE_GLOBAL", offset);
    case Opcode::OP_SET_GLOBAL:
        return constantInstruction("OP_SET_GLOBAL", offset);
    case Opcode::OP_GET_UPVALUE:
        return byteInstruction("OP_GET_UPVALUE", offset);
    case Opcode::OP_SET_UPVALUE:
        return byteInstruction("OP_SET_UPVALUE", offset);
    case Opcode::OP_GET_PROPERTY:
        return constantInstruction("OP_GET_PROPERTY", offset);
    case Opcode::OP_SET_PROPERTY:
        return constantInstruction("OP_SET_PROPERTY", offset);
    case Opcode::OP_GET_SUPER:
        return constantInstruction("OP_GET_SUPER", offset);
    case Opcode::OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case Opcode::OP_EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    case Opcode::OP_GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case Opcode::OP_LESS:
        return simpleInstruction("OP_LESS", offset);
    case Opcode::OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case Opcode::OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case Opcode::OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case Opcode::OP_NOT:
        return simpleInstruction("OP_NOT", offset);
    case Opcode::OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case Opcode::OP_PRINT:
        return simpleInstruction("OP_PRINT", offset);
    case Opcode::OP_JUMP:
        return jumpInstruction("OP_JUMP", 1, offset);
    case Opcode::OP_JUMP_IF_FALSE:
        return jumpInstruction("OP_JUMP_IF_FALSE", 1, offset);
    case Opcode::OP_LOOP:
        return jumpInstruction("OP_LOOP", -1, offset);
    case Opcode::OP_CALL:
        return byteInstruction("OP_CALL", offset);
    case Opcode::OP_INVOKE:
        return invokeInstruction("OP_INVOKE", offset);
    case Opcode::OP_INVOKE_SUPER:
        return invokeInstruction("OP_INVOKE_SUPER", offset);
    case Opcode::OP_CLOSURE:
    {
        offset++;
        auto constant = (*chunk)[offset++];
        printf("%-16s %4d ", "OP_CLOSURE", constant);
        auto value = chunk->getConstant(constant);
        printValue(value);
        std::cout << std::endl;
        auto function = asFunction(move(value));
        for (int i = 0; i < function->upvalueCount; i++)
        {
            int isLocal = function->chunk[offset++];
            int index = function->chunk[offset++];
            printf("%04d    |                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
        }
        return offset;
    }
    case Opcode::OP_CLOSE_UPVALUE:
        return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case Opcode::OP_CLASS:
        return constantInstruction("OP_CLASS", offset);
    case Opcode::OP_INHERIT:
        return simpleInstruction("OP_INHERIT", offset);
    case Opcode::OP_METHOD:
        return constantInstruction("OP_METHOD", offset);
    case Opcode::OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown op code " << std::to_string((*chunk)[offset]) << std::endl;
        return offset + 1;
    }
}

int Disassembler::simpleInstruction(string name, int offset)
{
    std::cout << name << std::endl;
    return offset + 1;
}

int Disassembler::constantInstruction(string name, int offset)
{
    auto constantIndex = (*chunk)[offset + 1];
    printf("%-16s %4d '", name.c_str(), constantIndex);
    printValue(chunk->getConstant(constantIndex));
    std::cout << std::endl;
    return offset + 2;
}

int Disassembler::constantLongInstruction(string name, int offset)
{
    int constantIndex = 0;
    for (int i = 0; i < 4; i++)
        constantIndex |= (*chunk)[offset + 1 + i] << (8 * i);
    printf("%-10s %10d '", name.c_str(), constantIndex);
    printValue(chunk->getConstant(constantIndex));
    std::cout << std::endl;
    return offset + 4;
}

int Disassembler::invokeInstruction(string name, int offset)
{
    auto constant = (*chunk)[offset + 1];
    auto argCount = (*chunk)[offset + 2];
    printf("%-16s (%d args) %4d '", name.c_str(), argCount, constant);
    printValue(chunk->getConstant(constant));
    printf("'\n");
    return offset + 3;
}

int Disassembler::byteInstruction(string name, int offset)
{
    auto slot = chunk->getCode()[offset + 1];
    printf("%-16s %4d\n", name.c_str(), slot);
    return offset + 2;
}

int Disassembler::jumpInstruction(string name, int sign, int offset)
{
    uint16_t jump = static_cast<uint16_t>((*chunk)[offset + 1] << 8);
    jump |= (*chunk)[offset + 2];
    printf("%-16s %4d -> %d\n", name.c_str(), offset, offset + 3 + sign * jump);
    return offset + 3;
}