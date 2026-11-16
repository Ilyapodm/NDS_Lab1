#include "avl_tree_utils.hpp"

#include <gtest/gtest.h>

using avl_test::TreeChecks;

TEST(AvlTree, InsertIntoEmptyTreeAndRejectDuplicate) {
    AVLTree<int> tree;
    TreeChecks::expect_valid(tree, 0);

    TreeChecks::insert_values(tree, {7});
    EXPECT_TRUE(tree.contains(7));

    EXPECT_FALSE(tree.insert(7));
    TreeChecks::expect_valid(tree, 1);
}

TEST(AvlTree, InsertInAscendingOrder) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {1, 2, 3, 4, 5, 6, 7, 8, 9});

    for (int value = 1; value <= 9; ++value) {
        EXPECT_TRUE(tree.contains(value)) << "key " << value;
    }
}

TEST(AvlTree, FindExistingAndMissingKeys) {
    AVLTree<int> tree;
    EXPECT_FALSE(tree.find(4).has_value());

    TreeChecks::insert_values(tree, {4, 2, 6});
    for (int value : {2, 4, 6}) {
        const auto result = tree.find(value);
        ASSERT_TRUE(result.has_value()) << "key " << value;
        EXPECT_EQ(result->get(), value);
    }
    EXPECT_FALSE(tree.find(5).has_value());
}

TEST(AvlTree, EraseOneChildLeafTwoChildrenAndRoot) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {8, 4, 12, 2, 6, 10, 14, 1});

    // key 2 has only child 1, key 6 is a leaf, and key 12 has two children
    EXPECT_TRUE(tree.erase(2));
    TreeChecks::expect_valid(tree, 7);
    EXPECT_FALSE(tree.contains(2));
    EXPECT_TRUE(tree.contains(1));

    EXPECT_TRUE(tree.erase(6));
    TreeChecks::expect_valid(tree, 6);
    EXPECT_FALSE(tree.contains(6));

    EXPECT_TRUE(tree.erase(12));
    TreeChecks::expect_valid(tree, 5);
    EXPECT_FALSE(tree.contains(12));

    TreeChecks::expect_root(tree, 8);
    EXPECT_TRUE(tree.erase(8));
    TreeChecks::expect_valid(tree, 4);
    EXPECT_FALSE(tree.contains(8));

    // size and the surviving keys together verify the remaining contents
    for (int value : {1, 4, 10, 14}) {
        EXPECT_TRUE(tree.contains(value)) << "key " << value;
    }
}

TEST(AvlTree, EraseMissingKey) {
    AVLTree<int> tree;
    EXPECT_FALSE(tree.erase(9));
    TreeChecks::expect_valid(tree, 0);

    TreeChecks::insert_values(tree, {8});
    EXPECT_FALSE(tree.erase(9));
    TreeChecks::expect_valid(tree, 1);
    EXPECT_TRUE(tree.contains(8));
}

TEST(AvlTree, BalanceLlRrLrRl) {
    // each insertion order triggers a different rotation
    for (const auto values : {std::initializer_list<int>{30, 20, 10},
                              {10, 20, 30}, {30, 10, 20}, {10, 30, 20}}) {
        SCOPED_TRACE(::testing::PrintToString(values));
        AVLTree<int> tree;
        TreeChecks::insert_values(tree, values);
        TreeChecks::expect_root(tree, 20);
    }
}

TEST(AvlTree, EraseRebalancesTwoAncestors) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {6, 4, 10, 2, 5, 8, 13, 1, 7, 12, 14, 11});
    TreeChecks::expect_root(tree, 6);

    // removing 5 rotates right at 4, shrinking that subtree and forcing a left rotation at 6
    EXPECT_TRUE(tree.erase(5));
    TreeChecks::expect_valid(tree, 11);
    TreeChecks::expect_root(tree, 10);
    EXPECT_FALSE(tree.contains(5));
    for (int value : {1, 2, 4, 6, 7, 8, 10, 11, 12, 13, 14}) {
        EXPECT_TRUE(tree.contains(value)) << "key " << value;
    }
}

TEST(AvlTree, ClearAndReuseTree) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {2, 1, 3});

    tree.clear();
    TreeChecks::expect_valid(tree, 0);
    EXPECT_FALSE(tree.find(2).has_value());

    tree.clear();
    TreeChecks::expect_valid(tree, 0);

    TreeChecks::insert_values(tree, {4});
    EXPECT_TRUE(tree.erase(4));
    TreeChecks::expect_valid(tree, 0);
}
