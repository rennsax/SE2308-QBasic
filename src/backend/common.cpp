#include "common.h"

void basic::log_error(std::ostream &err, LSize line, CSize column,
                      const std::string &msg) {
    err << "Error at line " << line << ", column " << column << ": " << msg;
}
