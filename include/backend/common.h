#ifndef BASIC_COMMON_H
#define BASIC_COMMON_H

#include <cstdint>
#include <iostream>

namespace basic {

/// Line size type
using LSize = std::uint32_t;
/// Column size type
using CSize = std::uint32_t;

/**
 * Logs an error message with the specified line number, column number, and
 * message.
 *
 * @param err The output stream to log the error message to.
 * @param line The line number where the error occurred.
 * @param column The column number where the error occurred.
 * @param msg The error message to be logged.
 */
void log_error(std::ostream &err, LSize line, CSize column,
               std::string_view msg);

} // namespace basic

#endif // BASIC_COMMON_H