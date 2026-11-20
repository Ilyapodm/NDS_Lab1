#pragma once

#include "tree/avl_tree.hpp"

#include "containers/queue.hpp"
#include "serialization/serialization_utils.hpp"

#include <algorithm>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <utility>

template <typename T, typename Compare>
AVLTree<T, Compare>::AVLTree(Compare comp) : comp_(std::move(comp)) {}

template <typename T, typename Compare>
AVLTree<T, Compare>::AVLTree(const AVLTree& other)
        : comp_(other.comp_)
        , root_(clone(other.root_))
        , size_(other.size_)
    {}

template <typename T, typename Compare>
AVLTree<T, Compare>::AVLTree(const AVLTree& other, Compare comp)
    : comp_(std::move(comp)), root_(clone(other.root_)), size_(other.size_) {}

template <typename T, typename Compare>
AVLTree<T, Compare>::AVLTree(AVLTree&& other)
    noexcept(std::is_nothrow_copy_constructible_v<Compare>) // conditional noexcept
    // copy the comparator before taking ownership so a failed copy leaves other intact
        : comp_(other.comp_)  // no difference between copy and move to std::less
        , root_(std::move(other.root_))
        , size_(std::exchange(other.size_, 0))  
    {}

template <typename T, typename Compare>
AVLTree<T, Compare>& AVLTree<T, Compare>::operator=(const AVLTree& other) {
    if (this != &other) {
        AVLTree copy(other);
        swap_state(copy);
    }
    return *this;
}

template <typename T, typename Compare>
AVLTree<T, Compare>& AVLTree<T, Compare>::operator=(AVLTree&& other) noexcept {
    if (this != &other) {
        swap_state(other);
        other.clear();
    }
    return *this;
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::swap_state(AVLTree& other) noexcept {
    // commit the comparator and its matching tree without an exception between them
    static_assert(std::is_nothrow_swappable_v<Compare>,
                  "AVLTree assignment requires a noexcept swap for Compare");
    using std::swap;
    swap(comp_, other.comp_);
    swap(root_, other.root_);
    swap(size_, other.size_);
}

template <typename T, typename Compare>
typename AVLTree<T, Compare>::node_link AVLTree<T, Compare>::clone(const node_link& link) {
    if (!link) {
        return nullptr;
    }

    const Node<T>& node = *link;
    auto copy = std::make_unique<Node<T>>(node.value);  // make_unique uses explicit constructor, OK
    copy->left = clone(node.left);
    copy->right = clone(node.right);
    copy->height = node.height;
    return copy;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::insert(const T& value) {
    if (!insert_impl(root_, value)) {
        return false;
    }
    ++size_;
    return true;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::insert(T&& value) {
    if (!insert_impl(root_, std::move(value))) {
        return false;
    }
    ++size_;
    return true;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::erase(const T& key) {
    if (!erase_impl(root_, key)) {
        return false;
    }
    --size_;
    return true;
}

template <typename T, typename Compare>
typename AVLTree<T, Compare>::find_result // show, that 'find_result' is a typename and from AVLTree
AVLTree<T, Compare>::find(const T& key) const {
    const node_link& link = find_impl(root_, key);
    if (!link) {
        return std::nullopt;
    }
    return std::cref(link->value);
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::contains(const T& key) const {
    return find(key).has_value();
}

template <typename T, typename Compare>
std::size_t AVLTree<T, Compare>::size() const noexcept {
    return size_;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::empty() const noexcept {
    return size_ == 0;
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::clear() noexcept {
    root_.reset();
    size_ = 0;
}

template <typename T, typename Compare>
std::size_t AVLTree<T, Compare>::height(const node_link& link) noexcept {
    return link ? link->height : 0;
}

template <typename T, typename Compare>
int AVLTree<T, Compare>::balance_factor(const Node<T>& node) noexcept {
    // if it's > 1 -> overlifted on the left side
    // < -1 -> overlifted on the right side
    return static_cast<int>(height(node.left)) - static_cast<int>(height(node.right));
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::update_height(Node<T>& node) noexcept {
    node.height = 1 + std::max(height(node.left), height(node.right));
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::rotate_left(node_link& link) noexcept {
    auto new_root = std::move(link->right);  // unplug right child (Y)
    link->right = std::move(new_root->left);  // link the left subtree of (Y)
    update_height(*link);  

    new_root->left = std::move(link);
    update_height(*new_root);
    link = std::move(new_root);
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::rotate_right(node_link& link) noexcept {
    auto new_root = std::move(link->left);
    link->left = std::move(new_root->right);
    update_height(*link);

    new_root->right = std::move(link);
    update_height(*new_root);
    link = std::move(new_root);
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::rebalance(node_link& link) noexcept {
    update_height(*link);
    const int balance = balance_factor(*link);

    // left is overlifted
    if (balance > 1) {
        // right is overlifted
        if (balance_factor(*link->left) < 0) {
            rotate_left(link->left);
        }
        rotate_right(link);
    } else if (balance < -1) {
        if (balance_factor(*link->right) > 0) {
            rotate_right(link->right);
        }
        rotate_left(link);
    }
}

template <typename T, typename Compare>
typename AVLTree<T, Compare>::node_link
AVLTree<T, Compare>::detach_min(node_link& link) noexcept {
    // there is no smaller values
    if (!link->left) {  
        auto min_node = std::move(link);  // min_node now owe the link's nodes (subtree)
        link = std::move(min_node->right);
        return min_node;
    }

    auto min_node = detach_min(link->left);
    rebalance(link);
    return min_node;
}

template <typename T, typename Compare>
const typename AVLTree<T, Compare>::node_link&
AVLTree<T, Compare>::find_impl(const node_link& link, const T& key) const {
    if (!link) {
        return link;
    }

    const Node<T>& node = *link;

    if (comp_(key, node.value)) {
        return find_impl(node.left, key);
    }
    if (comp_(node.value, key)) {
        return find_impl(node.right, key);
    }

    return link;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::insert_impl(node_link& link, const T& value) {
    // if there is no root
    if (!link) {
        link = std::make_unique<Node<T>>(value);
        return true;
    }

    const T& node_value = link->value;  // to make code readeable
    if (comp_(value, node_value)) {
        if (!insert_impl(link->left, value)) {
            return false;
        }
    } else if (comp_(node_value, value)) {
        if (!insert_impl(link->right, value)) {
            return false;
        }
    } else {
        return false;
    }

    rebalance(link);
    return true;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::insert_impl(node_link& link, T&& value) {
    // move the value only after finding an empty link
    if (!link) {
        link = std::make_unique<Node<T>>(std::move(value));
        return true;
    }

    const T& node_value = link->value;  // to make code readeable
    if (comp_(value, node_value)) {
        if (!insert_impl(link->left, std::move(value))) {
            return false;
        }
    } else if (comp_(node_value, value)) {
        if (!insert_impl(link->right, std::move(value))) {
            return false;
        }
    } else {
        return false;
    }

    rebalance(link);
    return true;
}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::erase_impl(node_link& link, const T& key) {
    if (!link) {
        return false;
    }

    const T& node_value = link->value;  // to make code readeable
    if (comp_(key, node_value)) {
        if (!erase_impl(link->left, key)) {
            return false;
        }
    } else if (comp_(node_value, key)) {
        if (!erase_impl(link->right, key)) {
            return false;
        }
    } else {
        auto removed = std::move(link);

        // if only one child
        if (!removed->left) {
            link = std::move(removed->right);
            return true;
        }
        if (!removed->right) {
            link = std::move(removed->left);
            return true;
        }

        // if 2 children
        auto successor = detach_min(removed->right);
        successor->left = std::move(removed->left);
        successor->right = std::move(removed->right);
        link = std::move(successor);
    }

    rebalance(link);
    return true;
}

template <typename T, typename Compare>
template <typename Visitor>
    requires std::invocable<Visitor&, const T&>
void AVLTree<T, Compare>::for_each(TraversalOrder order, Visitor&& visitor) const {
    // validate before visiting any values, including when the tree is empty
    switch (order) {
    case TraversalOrder::KLP:
    case TraversalOrder::KPL:
    case TraversalOrder::LPK:
    case TraversalOrder::LKP:
    case TraversalOrder::PLK:
    case TraversalOrder::PKL:
        break;
    default:
        throw std::invalid_argument("invalid traversal order");
    }

    if (root_) {
        traverse_impl(*root_, order, visitor);
    }
}

template <typename T, typename Compare>
template <typename Visitor>
void AVLTree<T, Compare>::traverse_impl(const Node<T>& node,
                                      TraversalOrder order,
                                      Visitor& visitor) {

    const bool left_first = order == TraversalOrder::KLP
                         || order == TraversalOrder::LKP
                         || order == TraversalOrder::LPK;
    
    const node_link& first = left_first ? node.left : node.right;
    const node_link& second = left_first ? node.right : node.left;

    // the position of k selects when to visit the value between the two subtrees
    if (order == TraversalOrder::KLP || order == TraversalOrder::KPL) {
        std::invoke(visitor, node.value); // invoke because supports ptrs to methods
    }
    if (first) {
        traverse_impl(*first, order, visitor);
    }
    if (order == TraversalOrder::LKP || order == TraversalOrder::PKL) {
        std::invoke(visitor, node.value);
    }
    if (second) {
        traverse_impl(*second, order, visitor);
    }
    if (order == TraversalOrder::LPK || order == TraversalOrder::PLK) {
        std::invoke(visitor, node.value);
    }
}

template <typename T, typename Compare>
template <typename Visitor>
    requires std::invocable<Visitor&, const T&>
void AVLTree<T, Compare>::for_each_bfs(Visitor&& visitor) const {
    if (!root_) {
        return;
    }

    // queue entries refer to nodes that remain owned by the tree
    Queue<std::reference_wrapper<const Node<T>>> pending;
    pending.push(std::cref(*root_));

    while (!pending.empty()) {
        const Node<T>& node = pending.front().get();
        // popping destroys the queue entry while the tree node stays alive
        pending.pop();
        std::invoke(visitor, node.value);

        if (node.left) {
            pending.push(std::cref(*node.left));
        }
        if (node.right) {
            pending.push(std::cref(*node.right));
        }
    }
}

template <typename T, typename Compare>
template <typename Mapper>
    requires std::invocable<Mapper&, const T&>
          && std::same_as<std::invoke_result_t<Mapper&, const T&>, T>
AVLTree<T, Compare> AVLTree<T, Compare>::map(Mapper&& mapper) const {
    AVLTree<T, Compare> result(comp_);
    for_each(TraversalOrder::LKP, [&](const T& value) {
        // insertion restores order and merges equivalent mapped values
        result.insert(std::invoke(mapper, value));
    });

    return result;
}

template <typename T, typename Compare>
template <typename Predicate>
    requires std::predicate<Predicate&, const T&>
AVLTree<T, Compare> AVLTree<T, Compare>::where(Predicate&& predicate) const {
    AVLTree<T, Compare> result(comp_);
    for_each(TraversalOrder::LKP, [&](const T& value) {
        if (std::invoke(predicate, value)) {
            result.insert(value);
        }
    });

    return result;
}

template <typename T, typename Compare>
template <typename Accumulator, typename Reducer>
Accumulator AVLTree<T, Compare>::reduce(Accumulator initial, Reducer&& reducer,
                                       TraversalOrder order) const {
    for_each(order, [&](const T& value) {
        // pass ownership of the current accumulator to support move-only values
        initial = std::invoke(reducer, std::move(initial), value);
    });
    return initial;
}

template <typename T, typename Compare>
std::string AVLTree<T, Compare>::serialize() const {
    std::ostringstream output;
    // make numbers independent of the process locale, using a decimal point
    output.imbue(std::locale::classic());
    serialize_impl(root_, output);
    // include failures while writing separators and null markers
    if (!output) {
        throw std::invalid_argument("failed to write tree");
    }
    std::string data = output.str();
    // every recursive write ends in a space, including the final null marker
    data.pop_back();
    return data;
}

template <typename T, typename Compare>
void AVLTree<T, Compare>::serialize_impl(const node_link& link, std::ostream& output) {
    if (!link) {
        output << "# ";
        return;
    }

    // preorder records the value before both child slots
    // null markers are essential because values alone do not describe the shape
    serialization::write_value(output, link->value);
    output << ' ';
    serialize_impl(link->left, output);
    serialize_impl(link->right, output);
}

template <typename T, typename Compare>
AVLTree<T, Compare> AVLTree<T, Compare>::deserialize(const std::string& data, Compare comp) {
    AVLTree<T, Compare> result(std::move(comp));
    std::istringstream input(data);
    input.imbue(std::locale::classic());

    // construct a separate tree and publish it only after all checks succeed
    result.root_ = result.deserialize_impl(input, std::nullopt, std::nullopt, 0);
    serialization::check_input_end(input);
    return result;
}

template <typename T, typename Compare>
typename AVLTree<T, Compare>::node_link AVLTree<T, Compare>::deserialize_impl(
    std::istream& input,
    std::optional<std::reference_wrapper<const T>> lower,
    std::optional<std::reference_wrapper<const T>> upper,
    std::size_t depth) {
    input >> std::ws;
    const int next = input.peek();
    if (next == std::char_traits<char>::eof()) {
        throw std::invalid_argument("incomplete tree");
    }
    if (next == '#') {
        input.get();
        // a marker must stand alone, so reject inputs such as "#abc"
        serialization::check_value_end(input);
        return nullptr;
    }

    // an avl of height h has at least 2^(h / 2) - 1 nodes
    // twice the bit count of size_t is a safe upper bound on representable height
    // reject deeper input before recursion since malformed input need not be balanced
    constexpr std::size_t max_depth = 2 * std::numeric_limits<std::size_t>::digits;
    if (depth >= max_depth) {
        throw std::invalid_argument("tree is too deep for an AVL tree");
    }

    T value = serialization::read_value<T>(input);
    // bounds come from all ancestors, which also rules out equivalent keys
    if ((lower && !comp_(lower->get(), value))
        || (upper && !comp_(value, upper->get()))) {
        throw std::invalid_argument("invalid BST order or duplicate key");
    }

    auto node = std::make_unique<Node<T>>(std::move(value));
    // node stays alive while descendants use its value as a non-owning bound
    node->left = deserialize_impl(input, lower, std::cref(node->value), depth + 1);
    node->right = deserialize_impl(input, std::cref(node->value), upper, depth + 1);

    // children are complete, so their computed heights can validate this node
    // do not rotate here since that would silently change an invalid input tree
    update_height(*node);
    if (balance_factor(*node) < -1 || balance_factor(*node) > 1) {
        throw std::invalid_argument("invalid AVL balance");
    }
    ++size_;
    return node;
}

template <typename T, typename Compare>
const Compare& AVLTree<T, Compare>::key_comp() const noexcept {
    return comp_;
}

template <typename T, typename Compare>
template <typename Predicate>
    requires std::predicate<Predicate&, const T&>
bool AVLTree<T, Compare>::all_of(Predicate&& predicate) const {
    return all_of_impl(root_, predicate);
}

template <typename T, typename Compare>
template <typename Predicate>
bool AVLTree<T, Compare>::all_of_impl(const node_link& link, Predicate& predicate) {
    if (!link) {
        return true;
    }

    const Node<T>& node = *link;  // for chosing 'const T&' override in the predicate
    // && skips both the remaining values and their recursive calls after failure
    return all_of_impl(node.left, predicate)
        && std::invoke(predicate, node.value)
        && all_of_impl(node.right, predicate);
}

template <typename T, typename Compare>
AVLTree<T, Compare> AVLTree<T, Compare>::get_subtree(const T& key) const {
    // borrow the owning link only for the duration of this operation
    const node_link& link = find_impl(root_, key);
    if (!link) {
        throw std::out_of_range("subtree root was not found");
    }

    AVLTree<T, Compare> result(comp_);
    // clone preserves the shape and heights and gives the result its own nodes
    result.root_ = clone(link);
    // subtree sizes are not stored in nodes, so count the copied values in one traversal
    result.for_each(TraversalOrder::LKP, [&](const T&) {
        ++result.size_;
    });
    return result;
}
