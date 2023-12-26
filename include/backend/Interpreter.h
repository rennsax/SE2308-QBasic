#ifndef BASIC_INTERPRETER_H
#define BASIC_INTERPRETER_H

#include "Fragment.h"
#include <iostream>

namespace basic {

class Interpreter {
public:
    Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                std::ostream &err, std::istream &is);

    /**
     * @brief Construct a new Interpreter object
     *
     * @param frag The code fragment to be interpreted.
     * @param out, err The output and error streams.
     * @param is The input stream.
     * @param show_prompt The prompt function. Invoked when `INPUT` clauses are
     * interpreted.
     */
    Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                std::ostream &err, std::istream &is,
                std::function<void()> show_prompt);

    ~Interpreter() = default;

    // No copy or move.
    Interpreter(const Interpreter &other) = delete;
    Interpreter(Interpreter &&other) = delete;
    Interpreter &operator=(const Interpreter &other) = delete;
    Interpreter &operator=(Interpreter &&other) = delete;

    void interpret();

private:
    /// The Basic code to be interpreted.
    std::shared_ptr<Fragment> frag{};

    /// Standard output, error, and input streams that are bind to the Basic
    /// interpreter.
    std::ostream &out, &err;
    std::istream &is;

    /// Invoked when `INPUT` is interpreted.
    std::function<void()> show_prompt{};
};

}; // namespace basic

#endif // BASIC_INTERPRETER_H