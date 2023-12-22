#ifndef BASIC_FRAGMENT_H
#define BASIC_FRAGMENT_H

#include <cstdint>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace basic {

class Fragment {
    /// Line size type
    using LSize = std::uint32_t;

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
     * Inserts a line at the specified position.
     *
     * @param line The line to be inserted.
     * @note Insertion respects the raw content of line, i.e. it will never
     * strip the tailing newline character (\n).
     * @param pos The position at which the line should be inserted.
     * @return True if the line was successfully inserted, false otherwise.
     */
    bool insert(const std::string &line, LSize pos) noexcept;
    bool insert(const std::string &line) noexcept;

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
     * @param pos The position of the line to retrieve.
     * @return An optional string containing the line at the specified position,
     * or an empty optional if the position is invalid.
     */
    std::optional<std::string> get_line(LSize pos) const noexcept;

    LSize size() const noexcept {
        return lines_.size();
    }

private:
    using LineContainer = std::list<std::string>;
    using LineIter = LineContainer::iterator;
    using LineConstIter = LineContainer::const_iterator;

    /// The fragment lines.
    LineContainer lines_{};

    /// Expected to be immutable.
    std::string delimiter_{"\n"};

    LineConstIter get_iter_(LSize pos) const;
};

} // namespace basic

#endif // BASIC_FRAGMENT_H