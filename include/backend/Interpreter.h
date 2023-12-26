#ifndef BASIC_INTERPRETER_H
#define BASIC_INTERPRETER_H

#include "Fragment.h"
#include <iostream>

namespace basic {

class Interpreter {
public:
    /**
     * @brief Construct a new Interpreter object
     *
     * @param frag The code fragment to be interpreted.
     * @param out, err The output and error streams.
     * @param input_action The action to be performed when `INPUT` statement is
     * interpreted.
     */
    Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                std::ostream &err, std::function<std::string()> input_action);

    /**
     * @b A convenient overload.
     *
     * The input action is initialized as: read a line from the standard is each
     * time.
     */
    Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                std::ostream &err, std::istream &is);

    ~Interpreter() = default;

    // No copy or move.
    Interpreter(const Interpreter &other) = delete;
    Interpreter(Interpreter &&other) = delete;
    Interpreter &operator=(const Interpreter &other) = delete;
    Interpreter &operator=(Interpreter &&other) = delete;

    /**
     * @brief Trigger the interpretation of the stored fragment.
     *
     * Output and error informations are written to the corresponding streams.
     */
    void interpret();

    std::string show_ast() const;

private:
    /// The Basic code to be interpreted.
    std::shared_ptr<Fragment> frag{};

    /// Standard output, error streams that are bind to the Basic
    /// interpreter.
    std::ostream &out, &err;

    std::function<std::string()> input_action;
};

}; // namespace basic

#endif // BASIC_INTERPRETER_H