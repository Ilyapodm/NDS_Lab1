#pragma once

#include "tree/avl_tree.hpp"

#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>

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

    // equivalent mapped keys keep the first value in the source comparator order
    template <typename Mapper>
        requires std::invocable<Mapper&, const T&>
              && std::same_as<std::invoke_result_t<Mapper&, const T&>, T>
    Set<T, Compare> map(Mapper&& mapper) const;

    template <typename Predicate>
        requires std::predicate<Predicate&, const T&>
    Set<T, Compare> where(Predicate&& predicate) const;

    template <typename Accumulator, typename Reducer>
    Accumulator reduce(Accumulator initial, Reducer&& reducer) const;

    // save the internal tree shape and complete stored values
    std::string serialize() const;

    // build a new set using the supplied comparator to validate the stored order
    static Set<T, Compare> deserialize(const std::string& data, Compare comp = Compare{});

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
    explicit Set(AVLTree<T, Compare>&& tree);

    AVLTree<T, Compare> tree_;
};

#include "set.tpp"
