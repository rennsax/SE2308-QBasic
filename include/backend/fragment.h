#ifndef BASIC_FRAGMENT_H
#define BASIC_FRAGMENT_H

#include "common.h"
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace basic {

class Fragment {

public:
    explicit Fragment();

    /**
     * @brief Represents a fragment of a string.
     *
     * This class is used to represent a fragment of a string, delimited by a
     * specified delimiter.
     *
     * @param delimiter The delimiter used to separate each line. Typically "\n"
     * (LF) or "\r\n"(CRLF). Default to be "\n".
     */
    explicit Fragment(std::string_view delimiter);

    /**
     * @b Reads a Fragment object from the given input stream.
     *
     * @param is The input stream to read from.
     * @return The Fragment object read from the input stream.
     */
    static Fragment read_stream(std::istream &is);

    Fragment(const Fragment &other) noexcept;
    Fragment(Fragment &&other) noexcept;

    Fragment &operator=(const Fragment &other) = delete;
    Fragment &operator=(Fragment &&other) = delete;

    ~Fragment() = default;

    /**
     * @brief Inserts a line at the specified position.
     *
     * Refuse to insert if:
     * 1. there is already a line at pos; 2. the line is empty; 3. pos == 1
     *
     * @param pos The position at which the line should be inserted.
     * @param line The line to be inserted.
     * @note Insertion respects the raw content of line, i.e. it will never
     * strip the tailing newline character (\n).
     * @return True if the line was successfully inserted, false otherwise.
     */
    bool insert(LSize pos, const std::string &line) noexcept;

    /**
     * @brief Appends a line to the fragment.
     *
     * The new line number is: (last_line_num / 10 + 1) * 10
     *
     * @param line The line to append.
     * @return False iff the given line is empty.
     */
    bool append(const std::string &line) noexcept;

    /**
     * @brief Removes a line at the specified position.
     *
     * @param pos The position at which the line should be removed.
     * @return True if the element is successfully removed, false otherwise.
     */
    bool remove(LSize pos) noexcept;

    /**
     * @brief Concatenate each line, while prepending their line numbers.
     *
     * @return std::string Render result, can be shown on the UI.
     */
    std::string render() const;

    /**
     * @brief Concatenate each line into the STL string stream.
     *
     * Get the raw content of the fragment.
     *
     * @return std::stringstream The fragment stream.
     */
    std::stringstream get_frag_stream() const;

    /**
     * Retrieves the line at the specified position.
     *
     * @param line_num The position of the line to retrieve.
     * @return An optional string containing the line at the specified position,
     * or an empty optional if the position is invalid.
     */
    std::optional<std::string> get_line(LSize line_num) const noexcept;

    LSize size() const noexcept {
        return lines_.size();
    }

    /**
     * @brief Get the line number at the "absolute" position of the fragment.
     *
     * @param pos The absolute position of the line to retrieve. The index
     * begins with 1, which corresponds to the error reporting in ANTLR.
     * @return std::optional<LSize> If pos == 0 or pos >= size(), return an
     * empty result.
     */
    std::optional<LSize> get_line_number_at(std::size_t pos) const noexcept;

private:
    using LineContainer = std::map<LSize, std::string>;
    using LineIter = LineContainer::iterator;
    using LineConstIter = LineContainer::const_iterator;

    /// The fragment lines.
    LineContainer lines_{};

    /// Expected to be immutable.
    std::string delimiter_{"\n"};

    LineConstIter get_iter_(LSize pos) const;
    bool is_valid_iter_(LineConstIter iter) const noexcept;
    bool is_valid_line_(LSize pos) const noexcept;
};

} // namespace basic

#endif // BASIC_FRAGMENT_H