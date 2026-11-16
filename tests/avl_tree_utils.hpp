#pragma once

#include "tree/avl_tree.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <optional>

namespace avl_test {

class TreeChecks {
public:
    static void expect_valid(const AVLTree<int>& tree, std::size_t expected_size) {
        std::size_t node_count = 0;
        check_subtree(tree.root_, std::nullopt, std::nullopt, node_count);

        EXPECT_EQ(tree.size_, node_count);
        EXPECT_EQ(tree.size(), expected_size);
        EXPECT_EQ(tree.empty(), expected_size == 0);
    }

    static void expect_root(const AVLTree<int>& tree, int value) {
        ASSERT_TRUE(tree.root_);
        EXPECT_EQ(tree.root_->value, value);
    }

    static void insert_values(AVLTree<int>& tree, std::initializer_list<int> values) {
        auto expected_size = tree.size();
        for (int value : values) {
            ASSERT_TRUE(tree.insert(value)) << "key " << value;
            expect_valid(tree, ++expected_size);
        }
    }

private:
    static std::size_t check_subtree(const std::unique_ptr<Node<int>>& link,
                                     std::optional<int> lower,
                                     std::optional<int> upper,
                                     std::size_t& node_count) {
        if (!link) {
            return 0;
        }

        const Node<int>& node = *link;
        ++node_count;

        // carry bounds from all ancestors to check the full bst order
        if (lower) {
            EXPECT_LT(*lower, node.value);
        }
        if (upper) {
            EXPECT_LT(node.value, *upper);
        }

        // compute heights from the actual structure before checking stored heights
        const auto left_height = check_subtree(node.left, lower, node.value, node_count);
        const auto right_height = check_subtree(node.right, node.value, upper, node_count);
        const auto taller = std::max(left_height, right_height);

        EXPECT_LE(taller - std::min(left_height, right_height), 1u) << "key " << node.value;
        EXPECT_EQ(node.height, 1 + taller) << "key " << node.value;
        return 1 + taller;
    }
};

}  // namespace avl_test
