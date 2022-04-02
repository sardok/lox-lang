#include <chrono>
#include "value.hpp"

Value clockNative(int argCount, Value *args)
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return numberValue(static_cast<double>(ms));
}