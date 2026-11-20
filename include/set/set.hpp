#pragma once

#include "tree/avl_tree.hpp"

#include <cstddef>
#include <functional>

template <typename T, typename Compare = std::less<T>>
class Set {
public:
    using find_result = typename AVLTree<T, Compare>::find_result;

    Set() = default;
    explicit Set(Compare comp);

    bool insert(const T& value);
    bool insert(T&& value);
    bool erase(const T& key);
    bool contains(const T& key) const;
    find_result find(const T& key) const;
    std::size_t size() const noexcept;
    bool empty() const noexcept;
    void clear() noexcept;

    template <typename Visitor>
    void for_each(Visitor&& visitor) const;

    // binary operations require comparators defining the same order and key equivalence
    // union and intersection keep the left operand's value for equivalent keys
    Set<T, Compare> union_with(const Set& other) const;
    Set<T, Compare> intersection_with(const Set& other) const;
    Set<T, Compare> difference(const Set& other) const;
    bool is_subset_of(const Set& other) const;
    bool operator==(const Set& other) const;
    bool operator!=(const Set& other) const;

private:
    Set(const Set& other, Compare comp);

    AVLTree<T, Compare> tree_;
};

#include "set.tpp"
