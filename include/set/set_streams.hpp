#pragma once

#include "set.hpp"

#include <ostream>

template <typename T, typename Compare>
std::ostream& operator<<(std::ostream& output, const Set<T, Compare>& set) {
    output << '{';
    bool first = true;
    set.for_each([&](const T& value) {
        if (!first) {
            output << ", ";
        }
        output << value;
        first = false;
    });
    return output << '}';
}
