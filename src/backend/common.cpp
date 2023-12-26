#include "common.h"

void basic::log_error(std::ostream &err, LSize line, CSize column,
                      const std::string &msg) {
    err << "line " << line << ":" << column << ' ' << msg << '\n';
}
