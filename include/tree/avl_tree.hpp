#pragma once

#include "node.hpp"
#include "traversal_order.hpp"

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>

namespace avl_test {
class TreeChecks;
}

template <typename T, typename Compare = std::less<T>>
class AVLTree {
public:
    using find_result = std::optional<std::reference_wrapper<const T>>;

    AVLTree() = default;
    explicit AVLTree(Compare comp);

    AVLTree(const AVLTree& other);
    AVLTree(AVLTree&& other) noexcept(std::is_nothrow_copy_constructible_v<Compare>);

    // assignment requires a comparator with a nonthrowing swap
    AVLTree& operator=(const AVLTree& other);
    AVLTree& operator=(AVLTree&& other) noexcept;

    bool insert(const T& value);
    bool erase(const T& key);

    // The reference remains valid until its element is erased or destroyed
    find_result find(const T& key) const;
    bool contains(const T& key) const;

    std::size_t size() const noexcept;
    bool empty() const noexcept;
    void clear() noexcept;

    // the visitor must not modify the structure of this tree during traversal
    template <typename Visitor>
        requires std::invocable<Visitor&, const T&>  // can we call Visitor& with const T& argument
    void for_each(TraversalOrder order, Visitor&& visitor) const;

private:
    friend class avl_test::TreeChecks;  // for tests

    using node_link = std::unique_ptr<Node<T>>;

    Compare comp_{};
    node_link root_;
    std::size_t size_ = 0;

    void swap_state(AVLTree& other) noexcept;
    static node_link clone(const node_link& link);

    static std::size_t height(const node_link& link) noexcept;  // check a possibly empty link without replacing it
    static int balance_factor(const Node<T>& node) noexcept;  // read an existing node and its child heights
    static void update_height(Node<T>& node) noexcept;  // update an existing node in place
    static void rotate_left(node_link& link) noexcept;  // replace the caller's subtree root
    static void rotate_right(node_link& link) noexcept;  // replace the caller's subtree root
    static void rebalance(node_link& link) noexcept;  // update the subtree height and rotate if needed
    static node_link detach_min(node_link& link) noexcept;  // remove the minimum node and return ownership

    find_result find_impl(const node_link& link, const T& key) const;
    bool insert_impl(node_link& link, const T& value);
    bool erase_impl(node_link& link, const T& key);

    template <typename Visitor>
    static void traverse_impl(const Node<T>& node, TraversalOrder order, Visitor& visitor);
};

#include "avl_tree.tpp"
