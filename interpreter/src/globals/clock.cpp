#include <chrono>
#include "clock.hpp"
#include "../object/number_object.hpp"

std::shared_ptr<Object> Clock::call(Interpreter *interpreter, std::vector<std::shared_ptr<Object>> arguments)
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return std::make_shared<NumberObject>(static_cast<double>(ms));
}

int Clock::arity() const
{
    return 0;
}

std::string Clock::toString() const
{
    return "<native clock fn>";
}