#include "chunk.hpp"
#include "value.hpp"

using std::move;
using std::size_t;
using std::uint32_t;
using std::uint8_t;

std::span<const uint8_t> Chunk::getCode() const
{
    return code;
}

void Chunk::write(Opcode instruction, int line)
{
    write(static_cast<uint8_t>(instruction), line);
}

void Chunk::write(uint8_t data, int line)
{
    code.push_back(data);
    lines.push_back(line);
}

void Chunk::write(uint32_t data, int line)
{
    auto val = data;
    for (int i = 0; i < 4; i++)
        write(static_cast<uint8_t>((data >> 8 * i) & 255), line);
}

int Chunk::addConstant(const Value value)
{
    constants.push_back(move(value));
    return constants.size() - 1;
}

const uint8_t Chunk::operator[](size_t index) const
{
    return code[index];
}

uint8_t &Chunk::operator[](size_t index)
{
    return code[index];
}

std::size_t Chunk::size() const
{
    return code.size();
}

Value Chunk::getConstant(int index) const
{
    return constants[index];
}

int Chunk::getLine(int index) const
{
    return lines[index];
}

uint8_t *Chunk::getCodeBaseAddr()
{
    return code.data();
}