#include "tests_utils/comparator_utils.hpp"
#include "tests_utils/avl_tree_utils.hpp"

#include <gtest/gtest.h>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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

TEST(AvlTree, CopyCreatesIndependentTrees) {
    AVLTree<int> source;
    TreeChecks::insert_values(source, {4, 2, 6, 1});

    AVLTree<int> copy(source);
    TreeChecks::expect_valid(copy, 4);
    TreeChecks::expect_root(copy, 4);
    for (int value : {1, 2, 4, 6}) {
        const auto original = source.find(value);
        const auto cloned = copy.find(value);
        ASSERT_TRUE(original && cloned);
        EXPECT_NE(&original->get(), &cloned->get());
    }

    AVLTree<int> assigned;
    TreeChecks::insert_values(assigned, {9});
    assigned = source;
    TreeChecks::expect_valid(assigned, 4);
    EXPECT_FALSE(assigned.contains(9));
    EXPECT_TRUE(assigned.erase(2));
    TreeChecks::expect_valid(assigned, 3);
    EXPECT_TRUE(source.contains(2));
    TreeChecks::expect_valid(source, 4);
}

TEST(AvlTree, MoveTransfersNodesAndLeavesSourceReusable) {
    AVLTree<int> source;
    TreeChecks::insert_values(source, {4, 2, 6});
    const auto element = source.find(2);
    ASSERT_TRUE(element);
    const int& original = element->get();

    AVLTree<int> moved(std::move(source));
    TreeChecks::expect_valid(source, 0);
    TreeChecks::expect_valid(moved, 3);
    TreeChecks::insert_values(source, {1});

    source = std::move(moved);
    TreeChecks::expect_valid(moved, 0);
    TreeChecks::expect_valid(source, 3);
    EXPECT_FALSE(source.contains(1));
    EXPECT_TRUE(source.contains(4));
    EXPECT_TRUE(source.contains(6));

    // the final reference still refers to the original node
    const auto transferred = source.find(2);
    ASSERT_TRUE(transferred);
    EXPECT_EQ(&transferred->get(), &original);
    TreeChecks::insert_values(moved, {8});
}

TEST(AvlTree, FailedKeyCopyPreservesBothTrees) {
    using avl_test::CopyCheckedKey;

    AVLTree<CopyCheckedKey> source;
    ASSERT_TRUE(source.insert(CopyCheckedKey{2}));
    ASSERT_TRUE(source.insert(CopyCheckedKey{1}));
    AVLTree<CopyCheckedKey> target;
    ASSERT_TRUE(target.insert(CopyCheckedKey{9}));

    CopyCheckedKey::copies_before_failure = 1;
    EXPECT_THROW(target = source, std::runtime_error);
    CopyCheckedKey::copies_before_failure = -1;

    EXPECT_EQ(source.size(), 2u);
    EXPECT_TRUE(source.contains(CopyCheckedKey{1}));
    EXPECT_TRUE(source.contains(CopyCheckedKey{2}));
    EXPECT_EQ(target.size(), 1u);
    EXPECT_TRUE(target.contains(CopyCheckedKey{9}));
}

TEST(AvlTree, ComparatorCopyFailurePreservesTreesAndMoveAssignmentSkipsCopy) {
    using avl_test::CopyCheckedOrder;

    AVLTree<int, CopyCheckedOrder> source(CopyCheckedOrder{true});
    ASSERT_TRUE(source.insert(2));
    ASSERT_TRUE(source.insert(1));
    AVLTree<int, CopyCheckedOrder> target(CopyCheckedOrder{false});
    ASSERT_TRUE(target.insert(9));

    CopyCheckedOrder::fail_on_copy = true;
    EXPECT_THROW(target = source, std::runtime_error);
    EXPECT_THROW((AVLTree<int, CopyCheckedOrder>(std::move(source))), std::runtime_error);
    EXPECT_EQ(target.size(), 1u);
    EXPECT_TRUE(target.contains(9));
    EXPECT_EQ(source.size(), 2u);
    EXPECT_TRUE(source.contains(1));
    EXPECT_TRUE(source.contains(2));

    // keep copies disabled to verify that move assignment only swaps comparator state
    target = std::move(source);
    CopyCheckedOrder::fail_on_copy = false;

    EXPECT_TRUE(source.empty());
    EXPECT_EQ(target.size(), 2u);
    EXPECT_TRUE(target.contains(1));
    EXPECT_TRUE(target.contains(2));
    EXPECT_FALSE(target.contains(9));
}

TEST(AvlTree, DfsVisitsAllSixOrders) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {4, 2, 6, 1, 3, 5, 7});

    const std::array orders{
        TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LPK,
        TraversalOrder::LKP, TraversalOrder::PLK, TraversalOrder::PKL
    };
    const std::array<std::vector<int>, 6> expected{{
        {4, 2, 1, 3, 6, 5, 7},
        {4, 6, 7, 5, 2, 3, 1},
        {1, 3, 2, 5, 7, 6, 4},
        {1, 2, 3, 4, 5, 6, 7},
        {7, 5, 6, 3, 1, 2, 4},
        {7, 6, 5, 4, 3, 2, 1}
    }};

    const auto& read_only = tree;
    for (std::size_t index = 0; index < orders.size(); ++index) {
        SCOPED_TRACE(index);
        std::vector<int> visited;
        read_only.for_each(orders[index], [&](const int& value) {
            visited.push_back(value);
        });
        EXPECT_EQ(visited, expected[index]);
    }
    TreeChecks::expect_valid(tree, 7);
}

TEST(AvlTree, BfsVisitsLevelsFromLeftToRight) {
    AVLTree<int> tree;
    // the missing rightmost leaf also checks a node with only one child
    TreeChecks::insert_values(tree, {4, 2, 6, 1, 3, 5});

    std::vector<int> visited;
    const auto& read_only = tree;
    read_only.for_each_bfs([&](const int& value) {
        visited.push_back(value);
    });
    EXPECT_EQ(visited, (std::vector<int>{4, 2, 6, 1, 3, 5}));
    TreeChecks::expect_valid(tree, 6);
}

TEST(AvlTree, TraversalsHandleEmptyTreeAndSingleNode) {
    AVLTree<int> tree;
    const std::array orders{
        TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LPK,
        TraversalOrder::LKP, TraversalOrder::PLK, TraversalOrder::PKL
    };
    std::vector<int> visited;
    auto visitor = [&](const int& value) { visited.push_back(value); };

    for (auto order : orders) {
        tree.for_each(order, visitor);
    }
    tree.for_each_bfs(visitor);
    EXPECT_TRUE(visited.empty());

    TreeChecks::insert_values(tree, {9});
    for (auto order : orders) {
        visited.clear();
        tree.for_each(order, visitor);
        EXPECT_EQ(visited, std::vector<int>{9});
    }
    visited.clear();
    tree.for_each_bfs(visitor);
    EXPECT_EQ(visited, std::vector<int>{9});
    TreeChecks::expect_valid(tree, 1);
}

TEST(AvlTree, TraversalsPropagateVisitorErrorsAndRejectInvalidOrder) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {2, 1, 3});
    int calls = 0;
    auto visitor = [&](const int&) {
        if (++calls == 2) {
            throw std::runtime_error("visitor failed");
        }
    };

    EXPECT_THROW(tree.for_each(TraversalOrder::LKP, visitor), std::runtime_error);
    EXPECT_EQ(calls, 2);
    calls = 0;
    EXPECT_THROW(tree.for_each_bfs(visitor), std::runtime_error);
    EXPECT_EQ(calls, 2);

    calls = 0;
    // enum values can be constructed explicitly from an invalid integer
    EXPECT_THROW(tree.for_each(static_cast<TraversalOrder>(99), visitor),
                 std::invalid_argument);
    EXPECT_EQ(calls, 0);
    TreeChecks::expect_valid(tree, 3);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_TRUE(tree.contains(2));
    EXPECT_TRUE(tree.contains(3));
}

TEST(AvlTree, MapRestoresOrderAndMergesDuplicates) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {-2, -1, 0, 1, 2});

    auto mapped = tree.map([](const int& value) { return -value; });
    TreeChecks::expect_valid(mapped, 5);
    for (int value : {-2, -1, 0, 1, 2}) {
        EXPECT_TRUE(mapped.contains(value));
    }

    auto squares = tree.map([](const int& value) { return value * value; });
    TreeChecks::expect_valid(squares, 3);
    for (int value : {0, 1, 4}) {
        EXPECT_TRUE(squares.contains(value));
    }

    EXPECT_TRUE(mapped.erase(1));
    TreeChecks::expect_valid(mapped, 4);
    EXPECT_TRUE(tree.contains(1));
    TreeChecks::expect_valid(tree, 5);
}

TEST(AvlTree, WhereSelectsSomeAllOrNoValues) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {2, 1, 3, 4});

    auto even = tree.where([](const int& value) { return value % 2 == 0; });
    TreeChecks::expect_valid(even, 2);
    EXPECT_TRUE(even.contains(2));
    EXPECT_TRUE(even.contains(4));

    auto all = tree.where([](const int&) { return true; });
    TreeChecks::expect_valid(all, 4);
    for (int value : {1, 2, 3, 4}) {
        EXPECT_TRUE(all.contains(value));
    }

    auto none = tree.where([](const int&) { return false; });
    TreeChecks::expect_valid(none, 0);

    EXPECT_TRUE(even.erase(2));
    TreeChecks::expect_valid(even, 1);
    EXPECT_TRUE(tree.contains(2));
    TreeChecks::expect_valid(tree, 4);
}

TEST(AvlTree, ReduceUsesInitialValueAndSelectedOrder) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {2, 1, 3});

    EXPECT_EQ(tree.reduce(10, [](int total, const int& value) {
        return total + value;
    }), 16);

    // concatenation makes traversal order observable
    auto join = [](std::string text, const int& value) {
        return text + std::to_string(value);
    };
    EXPECT_EQ(tree.reduce(std::string{}, join), "123");
    EXPECT_EQ(tree.reduce(std::string{}, join, TraversalOrder::KLP), "213");
    TreeChecks::expect_valid(tree, 3);
}

TEST(AvlTree, ReduceSupportsMoveOnlyAccumulator) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {2, 1, 3});

    auto sum = tree.reduce(std::make_unique<int>(10),
        [](std::unique_ptr<int> total, const int& value) {
            *total += value;
            return total;
        });
    ASSERT_TRUE(sum);
    EXPECT_EQ(*sum, 16);
    TreeChecks::expect_valid(tree, 3);
}

TEST(AvlTree, FunctionalOperationsHandleEmptyTreeWithoutCallingHandlers) {
    AVLTree<int> tree;
    int calls = 0;
    auto mapped = tree.map([&](const int& value) {
        ++calls;
        return value;
    });
    auto filtered = tree.where([&](const int&) {
        ++calls;
        return true;
    });
    const int result = tree.reduce(9, [&](int total, const int& value) {
        ++calls;
        return total + value;
    });

    EXPECT_EQ(calls, 0);
    EXPECT_EQ(result, 9);
    TreeChecks::expect_valid(mapped, 0);
    TreeChecks::expect_valid(filtered, 0);
    TreeChecks::expect_valid(tree, 0);
}

TEST(AvlTree, FunctionalOperationsPreserveSourceWhenHandlersThrow) {
    AVLTree<int> tree;
    TreeChecks::insert_values(tree, {2, 1, 3});

    // fail after one element so the result or accumulator is already partially built
    EXPECT_THROW(tree.map([](const int& value) {
        if (value == 2) {
            throw std::runtime_error("mapper failed");
        }
        return value;
    }), std::runtime_error);
    TreeChecks::expect_valid(tree, 3);

    EXPECT_THROW(tree.where([](const int& value) {
        if (value == 2) {
            throw std::runtime_error("predicate failed");
        }
        return true;
    }), std::runtime_error);
    TreeChecks::expect_valid(tree, 3);

    EXPECT_THROW(tree.reduce(0, [](int total, const int& value) {
        if (value == 2) {
            throw std::runtime_error("reducer failed");
        }
        return total + value;
    }), std::runtime_error);
    TreeChecks::expect_valid(tree, 3);
    for (int value : {1, 2, 3}) {
        EXPECT_TRUE(tree.contains(value));
    }
}

TEST(AvlTree, MapAndWherePreserveComparatorState) {
    AVLTree<int, avl_test::CopyCheckedOrder> tree(avl_test::CopyCheckedOrder{true});
    for (int value : {2, 1, 3}) {
        ASSERT_TRUE(tree.insert(value));
    }

    auto mapped = tree.map([](const int& value) { return value + 10; });
    auto filtered = tree.where([](const int& value) { return value > 1; });
    auto join = [](std::string text, const int& value) {
        return text + std::to_string(value) + " ";
    };
    EXPECT_EQ(mapped.reduce(std::string{}, join), "13 12 11 ");
    EXPECT_EQ(filtered.reduce(std::string{}, join), "3 2 ");
    EXPECT_EQ(tree.reduce(std::string{}, join), "3 2 1 ");
}

TEST(AvlTree, AllOfStopsAtFirstFailureAndAcceptsEmptyTree) {
    AVLTree<int> tree;
    int calls = 0;
    auto predicate = [&](const int& value) {
        ++calls;
        return value < 2;
    };
    EXPECT_TRUE(tree.all_of(predicate));
    EXPECT_EQ(calls, 0);

    TreeChecks::insert_values(tree, {2, 1, 3, 4});
    EXPECT_FALSE(tree.all_of(predicate));
    EXPECT_EQ(calls, 2);
    EXPECT_TRUE(tree.all_of([](const int& value) { return value > 0; }));
    TreeChecks::expect_valid(tree, 4);
}

TEST(AvlTree, CopyWithSelectedComparatorPreservesShapeAndIndependentNodes) {
    using comparator_test::CountingOrder;
    using Tree = AVLTree<int, CountingOrder>;
    int source_calls = 0;
    int selected_calls = 0;
    Tree source(CountingOrder{std::ref(source_calls)});
    for (int value : {4, 2, 6, 1}) {
        ASSERT_TRUE(source.insert(value));
    }

    Tree copy(source, CountingOrder{std::ref(selected_calls)});
    EXPECT_EQ(copy.size(), 4u);
    EXPECT_EQ(copy.serialize(), source.serialize());
    EXPECT_EQ(&copy.key_comp().calls.get(), &selected_calls);
    source_calls = 0;
    selected_calls = 0;
    EXPECT_TRUE(copy.contains(1));
    EXPECT_GT(selected_calls, 0);
    EXPECT_EQ(source_calls, 0);

    const auto original = source.find(4);
    const auto cloned = copy.find(4);
    ASSERT_TRUE(original && cloned);
    EXPECT_NE(&original->get(), &cloned->get());
    EXPECT_TRUE(copy.erase(2));
    EXPECT_EQ(copy.size(), 3u);
    EXPECT_TRUE(source.contains(2));

    Tree empty(CountingOrder{std::ref(source_calls)});
    Tree empty_copy(empty, CountingOrder{std::ref(selected_calls)});
    EXPECT_TRUE(empty_copy.empty());
    EXPECT_EQ(&empty_copy.key_comp().calls.get(), &selected_calls);
}

TEST(AvlTree, CopyWithSelectedComparatorCleansUpAfterCopyFailure) {
    using avl_test::CopyCheckedKey;
    using Tree = AVLTree<CopyCheckedKey>;
    Tree source;
    ASSERT_TRUE(source.insert(CopyCheckedKey{2}));
    ASSERT_TRUE(source.insert(CopyCheckedKey{1}));

    // the first node is already allocated when the second value copy fails
    CopyCheckedKey::copies_before_failure = 1;
    EXPECT_THROW((Tree(source, std::less<CopyCheckedKey>{})), std::runtime_error);
    CopyCheckedKey::copies_before_failure = -1;

    EXPECT_EQ(source.size(), 2u);
    EXPECT_TRUE(source.contains(CopyCheckedKey{1}));
    EXPECT_TRUE(source.contains(CopyCheckedKey{2}));
}
