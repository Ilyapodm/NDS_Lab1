#pragma once

#include <cctype>
#include <cmath>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace serialization {

// extraction of an integer may succeed for "12abc" by consuming only "12"
// require a separator or end of input so that such prefixes are rejected
inline void check_value_end(std::istream& input) {
    if (input.eof()) {
        return;
    }
    const int next = input.peek();
    if (next != std::char_traits<char>::eof()
        && !std::isspace(static_cast<unsigned char>(next))) {
        throw std::invalid_argument("unexpected characters after value");
    }
}

inline void check_input_end(std::istream& input) {
    // eofbit can already be set by the last successful numeric extraction
    if (!input.eof()) {
        input >> std::ws;
        if (input.peek() != std::char_traits<char>::eof()) {
            throw std::invalid_argument("unexpected trailing data");
        }
    }
}

template <typename T>
void write_value(std::ostream& output, const T& value) {
    if constexpr (std::is_floating_point_v<T>) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("non-finite value");
        }
        // enough significant digits to recover the original floating-point value
        output << std::setprecision(std::numeric_limits<T>::max_digits10);
    }

    if constexpr (std::is_same_v<T, std::string>) {
        // quoted escapes quotes and backslashes and preserves embedded whitespace
        output << std::quoted(value);
    } else {
        output << value;
    }
    if (!output) {
        throw std::invalid_argument("failed to write value");
    }
}

template <typename T>
T read_value(std::istream& input) {
    T value{};
    input >> std::ws;
    if constexpr (std::is_same_v<T, std::string>) {
        // quoted also accepts unquoted words, so explicitly require the opening quote
        if (input.peek() != '"' || !(input >> std::quoted(value))) {
            throw std::invalid_argument("expected a quoted string");
        }
    } else {
        if (!(input >> value)) {
            throw std::invalid_argument("invalid value");
        }
    }

    if constexpr (std::is_floating_point_v<T>) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("non-finite value");
        }
    }
    check_value_end(input);
    return value;
}

}  // namespace serialization
