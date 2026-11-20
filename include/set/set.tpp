#pragma once

#include "set/set.hpp"

template <typename T, typename Compare>
Set<T, Compare>::Set(Compare comp) : tree_(std::move(comp)) {}

template <typename T, typename Compare>
Set<T, Compare>::Set(const Set& other, Compare comp)
    : tree_(other.tree_, std::move(comp)) {}

template <typename T, typename Compare>
bool Set<T, Compare>::insert(const T& value) {
    return tree_.insert(value);
}

template <typename T, typename Compare>
bool Set<T, Compare>::insert(T&& value) {
    return tree_.insert(std::move(value));
}

template <typename T, typename Compare>
bool Set<T, Compare>::erase(const T& key) {
    return tree_.erase(key);
}

template <typename T, typename Compare>
bool Set<T, Compare>::contains(const T& key) const {
    return tree_.contains(key);
}

template <typename T, typename Compare>
typename Set<T, Compare>::find_result Set<T, Compare>::find(const T& key) const {
    return tree_.find(key);
}

template <typename T, typename Compare>
std::size_t Set<T, Compare>::size() const noexcept {
    return tree_.size();
}

template <typename T, typename Compare>
bool Set<T, Compare>::empty() const noexcept {
    return tree_.empty();
}

template <typename T, typename Compare>
void Set<T, Compare>::clear() noexcept {
    tree_.clear();
}

template <typename T, typename Compare>
template <typename Visitor>
void Set<T, Compare>::for_each(Visitor&& visitor) const {
    tree_.for_each(TraversalOrder::LKP, std::forward<Visitor>(visitor));
}

template <typename T, typename Compare>
Set<T, Compare> Set<T, Compare>::union_with(const Set& other) const {
    if (size() >= other.size()) {
        Set result(*this);
        other.for_each([&](const T& value) {
            result.insert(value);
        });
        return result;
    }

    Set result(other, tree_.key_comp());
    for_each([&](const T& value) {
        // replace the right operand's representative with the left operand's value
        result.erase(value);
        result.insert(value);
    });
    return result;
}

template <typename T, typename Compare>
Set<T, Compare> Set<T, Compare>::intersection_with(const Set& other) const {
    Set result(tree_.key_comp());

    if (size() <= other.size()) {
        for_each([&](const T& value) {
            if (other.contains(value)) {
                result.insert(value);
            }
        });
    } else {
        other.for_each([&](const T& value) {
            // even when scanning the right operand, take the value from the left
            const auto original = find(value);
            if (original) {
                result.insert(original->get());
            }
        });
    }
    return result;
}

template <typename T, typename Compare>
Set<T, Compare> Set<T, Compare>::difference(const Set& other) const {
    Set result(tree_.key_comp());
    for_each([&](const T& value) {
        if (!other.contains(value)) {
            result.insert(value);
        }
    });
    return result;
}

template <typename T, typename Compare>
bool Set<T, Compare>::is_subset_of(const Set& other) const {
    if (size() > other.size()) {
        return false;
    }
    return tree_.all_of([&](const T& value) {
        return other.contains(value);
    });
}

template <typename T, typename Compare>
bool Set<T, Compare>::operator==(const Set& other) const {
    return size() == other.size() && is_subset_of(other);
}

template <typename T, typename Compare>
bool Set<T, Compare>::operator!=(const Set& other) const {
    return !(*this == other);
}
