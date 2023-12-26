#ifndef BASIC_INTERPRETER_H
#define BASIC_INTERPRETER_H

#include "fragment.h"
#include <iostream>

namespace basic {

class Interpreter {
public:
    Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                std::ostream &err, std::istream &is);

    ~Interpreter() = default;

    // No copy or move.
    Interpreter(const Interpreter &other) = delete;
    Interpreter(Interpreter &&other) = delete;
    Interpreter &operator=(const Interpreter &other) = delete;
    Interpreter &operator=(Interpreter &&other) = delete;

    void interpret();

private:
    std::shared_ptr<Fragment> frag{};

    std::ostream &out, &err;
    std::istream &is;
};

}; // namespace basic

#endif // BASIC_INTERPRETER_H