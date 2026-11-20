#pragma once

#include <stdexcept>

namespace queue_test {

struct CopyCheckedValue {
    int value;
    inline static int copies_before_failure = -1;

    explicit CopyCheckedValue(int value) : value(value) {}

    CopyCheckedValue(const CopyCheckedValue& other) : value(other.value) {
        if (copies_before_failure == 0) {
            throw std::runtime_error("value copy failed");
        }
        if (copies_before_failure > 0) {
            --copies_before_failure;
        }
    }

    CopyCheckedValue(CopyCheckedValue&&) = default;
};

}  // namespace queue_test
