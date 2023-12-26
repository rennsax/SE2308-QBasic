#ifndef BASIC_COMMON_H
#define BASIC_COMMON_H

#include <cstdint>
#include <iostream>

namespace basic {

/// Line size type
using LSize = std::uint32_t;
using CSize = std::uint32_t;

void log_error(std::ostream &err, LSize line, CSize column,
               const std::string &msg);

} // namespace basic

#endif // BASIC_COMMON_H