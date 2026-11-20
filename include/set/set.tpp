#pragma once

#include "set/set.hpp"

template <typename T, typename Compare>
Set<T, Compare>::Set(Compare comp) : tree_(std::move(comp)) {}

template <typename T, typename Compare>
Set<T, Compare>::Set(const Set& other, Compare comp)
    : tree_(other.tree_, std::move(comp)) {}

template <typename T, typename Compare>
Set<T, Compare>::Set(AVLTree<T, Compare>&& tree)
    : tree_(std::move(tree)) {}

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

template <typename T, typename Compare>
template <typename Mapper>
    requires std::invocable<Mapper&, const T&>
          && std::same_as<std::invoke_result_t<Mapper&, const T&>, T>
Set<T, Compare> Set<T, Compare>::map(Mapper&& mapper) const {
    // tree map visits source keys in order and keeps the first equivalent result
    return Set(tree_.map(std::forward<Mapper>(mapper)));
}

template <typename T, typename Compare>
template <typename Predicate>
    requires std::predicate<Predicate&, const T&>
Set<T, Compare> Set<T, Compare>::where(Predicate&& predicate) const {
    return Set(tree_.where(std::forward<Predicate>(predicate)));
}

template <typename T, typename Compare>
template <typename Accumulator, typename Reducer>
Accumulator Set<T, Compare>::reduce(Accumulator initial, Reducer&& reducer) const {
    // set reduction always uses the tree's default inorder traversal
    return tree_.reduce(std::move(initial), std::forward<Reducer>(reducer));
}

template <typename T, typename Compare>
std::string Set<T, Compare>::serialize() const {
    // the tree already writes values in preorder with # for every empty child
    // delegating keeps quoting, numeric precision and format checks in one place
    return tree_.serialize();
}

template <typename T, typename Compare>
Set<T, Compare> Set<T, Compare>::deserialize(const std::string& data, Compare comp) {
    // restore a separate tree and validate the format, key order and avl balance
    // the comparator is supplied by the caller because its state is not in the string
    // any error leaves existing sets intact and releases partially built nodes
    // the private constructor takes ownership of the validated tree without cloning nodes
    return Set(AVLTree<T, Compare>::deserialize(data, std::move(comp)));
}
