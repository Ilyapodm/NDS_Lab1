#pragma once

#include <functional>

namespace comparator_test {

struct CountingOrder {
    std::reference_wrapper<int> calls;

    bool operator()(int left, int right) const {
        // count through a reference so comparator copies remain observable
        ++calls.get();
        return left < right;
    }
};

}  // namespace comparator_test
