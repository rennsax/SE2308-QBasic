#include "common.h"

void basic::log_error(std::ostream &err, LSize line, CSize column,
                      std::string_view msg) {
    err << "line " << line << ":" << column << ' ' << msg << '\n';
}
