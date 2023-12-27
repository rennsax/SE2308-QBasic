#include "Fragment.h"

namespace basic {

Fragment::Fragment() {
}

Fragment::Fragment(std::string_view delimiter) : delimiter_(delimiter) {
}

Fragment Fragment::read_stream(std::istream &is) {
    Fragment frag{"\n"};
    for (std::string line{}; std::getline(is, line);) {
        if (line.empty()) {
            // Escape empty lines.
            continue;
        }
        LSize line_num{};
        std::string content{};
        std::istringstream ss{line};
        ss >> line_num;
        ws(ss);
        std::getline(ss, content);
        if (!ss) {
            throw std::runtime_error{
                "cannot extract line number and line content"};
        }
        if (!frag.insert(line_num, content)) {
            throw std::runtime_error{"duplicated line number found!"};
        }
    }
    return frag;
}

Fragment::Fragment(const Fragment &other) noexcept
    : lines_(other.lines_), delimiter_(other.delimiter_) {
}

Fragment::Fragment(Fragment &&other) noexcept
    : lines_(std::move(other.lines_)), delimiter_(std::move(other.delimiter_)) {
}

bool Fragment::insert(LSize pos, const std::string &line) noexcept {
    if (line.empty()) {
        return false;
    }
    if (pos == 0) {
        return false;
    }
    if (is_valid_line_(pos)) {
        return false;
    }
    lines_.insert({pos, line});
    return true;
}

bool Fragment::append(const std::string &line) noexcept {
    if (line.empty()) {
        return false;
    }

    if (lines_.empty()) {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        lines_.insert({100, line});
        return true;
    }
    const auto last_line_num = lines_.rbegin()->first;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    auto next_line_num = (last_line_num / 10 + 1) * 10;
    lines_.insert({next_line_num, line});
    return true;
}

bool Fragment::remove(LSize pos) noexcept {
    return lines_.erase(pos) == 1;
}

std::string Fragment::render() const {
    return get_frag_stream().str();
}

std::stringstream Fragment::get_frag_stream() const {
    std::stringstream ss{};
    for (const auto &[line_num, line_str] : lines_) {
        ss << line_num << ' ' << line_str << delimiter_;
    }
    return ss;
}

std::optional<std::string> Fragment::get_line(LSize pos) const noexcept {
    if (!is_valid_line_(pos)) {
        return std::nullopt;
    }
    return get_iter_(pos)->second;
}

std::optional<LSize> Fragment::get_line_number_at(
    std::size_t pos) const noexcept {
    if (pos == 0 || pos > size()) {
        return std::nullopt;
    }
    return next(begin(lines_), static_cast<int>(pos - 1))->first;
}

auto Fragment::get_iter_(LSize pos) const -> LineConstIter {
    return lines_.find(pos);
}

bool Fragment::is_valid_iter_(LineConstIter iter) const noexcept {
    return iter != end(lines_);
}

bool Fragment::is_valid_line_(LSize pos) const noexcept {
    return is_valid_iter_(get_iter_(pos));
}

} // namespace basic