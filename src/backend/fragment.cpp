#include <fragment.h>

namespace basic {

Fragment::Fragment() {
}

Fragment::Fragment(std::string_view delimiter) : delimiter_(delimiter) {
}

Fragment::Fragment(const Fragment &other) noexcept
    : lines_(other.lines_), delimiter_(other.delimiter_) {
}

Fragment::Fragment(Fragment &&other) noexcept
    : lines_(std::move(other.lines_)), delimiter_(std::move(other.delimiter_)) {
}

bool Fragment::insert(const std::string &line, LSize pos) noexcept {
    if (pos > lines_.size()) {
        return false;
    }
    lines_.insert(get_iter_(pos), line);
    return true;
}

bool Fragment::insert(const std::string &line) noexcept {
    lines_.push_back(line);
    return true;
}

bool Fragment::remove(LSize pos) noexcept {
    if (pos >= lines_.size()) {
        return false;
    }
    lines_.erase(get_iter_(pos));
    return true;
}

std::string Fragment::render() const {
    std::stringstream ss{};
    LSize line_num = 0;
    for (auto it = begin(lines_); it != end(lines_); ++it, ++line_num) {
        ss << line_num << ' ' << *it << delimiter_;
    }
    return ss.str();
}

std::stringstream Fragment::get_frag_stream() const {
    std::stringstream ss{};
    for (auto it = begin(lines_); it != end(lines_); ++it) {
        ss << *it << delimiter_;
    }
    return ss;
}

std::optional<std::string> Fragment::get_line(LSize pos) const noexcept {
    if (pos >= lines_.size()) {
        return std::nullopt;
    }
    return *get_iter_(pos);
}

auto Fragment::get_iter_(LSize pos) const -> LineConstIter {
    return next(begin(lines_), pos);
}

} // namespace basic