#ifndef _DISASSEMBLER_HPP_
#define _DISASSEMBLER_HPP_
#include <string>
#include "chunk.hpp"

class Disassembler
{
public:
    explicit Disassembler(const Chunk *chunk) : chunk{chunk} {};
    void disassembleChunk(std::string);
    int disassembleInstruction(int);

private:
    int simpleInstruction(std::string, int);
    int constantInstruction(std::string, int);
    int constantLongInstruction(std::string, int);
    int invokeInstruction(std::string, int);
    int byteInstruction(std::string, int);
    int jumpInstruction(std::string, int, int);
    const Chunk *chunk;
};
#endif