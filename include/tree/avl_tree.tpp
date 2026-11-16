#include <algorithm>
#include <utility>

template <typename T, typename Compare>
AVLTree<T, Compare>::AVLTree(Compare comp) : comp_(std::move(comp)) {}

template <typename T, typename Compare>
bool AVLTree<T, Compare>::insert(const T& value) {
    if (!insert_impl(root_, value)) {
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
    return find_impl(root_, key);
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
typename AVLTree<T, Compare>::find_result  // show, that 'find_result' is a typename and from AVLTree
AVLTree<T, Compare>::find_impl(const node_link& link, const T& key) const {
    if (!link) {
        return std::nullopt;
    }

    const Node<T>& node = *link;

    if (comp_(key, node.value)) {
        return find_impl(node.left, key);
    }
    if (comp_(node.value, key)) {
        return find_impl(node.right, key);
    }

    return std::cref(node.value);
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
