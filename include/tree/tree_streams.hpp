#pragma once

#include "avl_tree.hpp"

#include <ostream>

template <typename T, typename Compare>
std::ostream& operator<<(std::ostream& output, const AVLTree<T, Compare>& tree) {
    // display values in comparator order while leaving serialization to serialize
    output << '[';
    bool first = true;
    tree.for_each(TraversalOrder::LKP, [&](const T& value) {
        if (!first) {
            output << ", ";
        }
        output << value;
        first = false;
    });
    return output << ']';
}
