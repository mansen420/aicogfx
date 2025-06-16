#pragma once

#include <cstdint>
#include <ostream>
#include <iostream>

namespace aico
{
    enum class operation_t : uint8_t
    {
        ADD,
        SUB,
        MUL,
        DIV
    };
    /**
     * @brief Evaluate arithmetic expression
     *
     * @return Result
     */
    /*don't discard this result*/[[nodiscard]]double 
    evaleq(double x, double y, operation_t op)
    /*this function does not throw exceptions*/noexcept;
    
    /**
     * @brief Prints arithmetic operation text to output stream, followed by a newline
     * character.
     */
    void puteq(double x, double y, operation_t op, std::ostream& stream = std::cout);
}
