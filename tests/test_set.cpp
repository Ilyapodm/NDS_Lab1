#include "comparator_utils.hpp"
#include "set/set.hpp"
#include "models/student.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <vector>

TEST(Set, BasicOperationsAndCopyAreDelegatedToTree) {
    Set<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_TRUE(set.insert(2));
    const int value = 1;
    EXPECT_TRUE(set.insert(value));
    EXPECT_FALSE(set.insert(2));
    EXPECT_EQ(set.size(), 2u);
    EXPECT_TRUE(set.contains(1));
    EXPECT_FALSE(set.find(3));
    const auto found = set.find(2);
    ASSERT_TRUE(found);
    EXPECT_EQ(found->get(), 2);

    Set<int> copy(set);
    EXPECT_TRUE(copy.erase(1));
    EXPECT_TRUE(set.contains(1));
    EXPECT_FALSE(copy.erase(1));
    copy.clear();
    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.size(), 0u);
}

TEST(Set, UnionIntersectionAndDifferenceHandleBothSizeOrders) {
    Set<int> left;
    for (int value : {1, 2, 3}) {
        left.insert(value);
    }
    Set<int> right;
    for (int value : {3, 4}) {
        right.insert(value);
    }

    const auto combined = left.union_with(right);
    EXPECT_EQ(combined.size(), 4u);
    EXPECT_EQ(combined, right.union_with(left));
    for (int value : {1, 2, 3, 4}) {
        EXPECT_TRUE(combined.contains(value));
    }

    const auto common = left.intersection_with(right);
    EXPECT_EQ(common.size(), 1u);
    EXPECT_TRUE(common.contains(3));
    EXPECT_EQ(common, right.intersection_with(left));

    const auto difference = left.difference(right);
    EXPECT_EQ(difference.size(), 2u);
    EXPECT_TRUE(difference.contains(1));
    EXPECT_TRUE(difference.contains(2));
    EXPECT_EQ(right.difference(left).size(), 1u);
    EXPECT_TRUE(right.difference(left).contains(4));

    EXPECT_EQ(left.size(), 3u);
    EXPECT_EQ(right.size(), 2u);
}

TEST(Set, EmptyDisjointAndSelfOperations) {
    Set<int> empty;
    Set<int> left;
    Set<int> right;
    left.insert(1);
    right.insert(2);

    EXPECT_EQ(left.union_with(empty), left);
    EXPECT_EQ(empty.union_with(left), left);
    EXPECT_TRUE(left.intersection_with(empty).empty());
    EXPECT_TRUE(empty.intersection_with(left).empty());
    EXPECT_TRUE(left.intersection_with(right).empty());
    EXPECT_EQ(left.difference(empty), left);
    EXPECT_TRUE(empty.difference(left).empty());
    EXPECT_EQ(left.union_with(left), left);
    EXPECT_EQ(left.intersection_with(left), left);
    EXPECT_TRUE(left.difference(left).empty());
    EXPECT_TRUE(empty.is_subset_of(left));
    EXPECT_FALSE(left.is_subset_of(empty));
}

TEST(Set, SubsetAndEqualityCompareKeysRegardlessOfTreeShape) {
    Set<int> left;
    Set<int> same;
    Set<int> subset;
    Set<int> different;
    for (int value : {1, 2, 3, 4, 5}) {
        left.insert(value);
    }
    for (int value : {3, 1, 5, 2, 4}) {
        same.insert(value);
    }
    subset.insert(2);
    subset.insert(4);
    different.insert(2);
    different.insert(9);

    EXPECT_EQ(left, same);
    EXPECT_TRUE(subset.is_subset_of(left));
    EXPECT_FALSE(left.is_subset_of(subset));
    EXPECT_FALSE(different.is_subset_of(left));
    EXPECT_NE(subset, different);
    EXPECT_NE(left, subset);
    EXPECT_EQ(Set<int>{}, Set<int>{});
}

TEST(Set, EquivalentKeysKeepLeftValuesInBothSizeBranches) {
    const Student original{2, "Left", "Student", 2005, 4.0};
    Set<Student> left;
    Set<Student> right;
    left.insert(original);
    right.insert(Student{2, "Right", "Student", 2004, 3.0});

    // equal ids make the sets equal even though the stored records differ
    EXPECT_EQ(left, right);
    auto combined = left.union_with(right);
    auto common = left.intersection_with(right);
    ASSERT_TRUE(combined.find(original));
    ASSERT_TRUE(common.find(original));
    EXPECT_EQ(combined.find(original)->get(), original);
    EXPECT_EQ(common.find(original)->get(), original);

    right.insert(Student{3, "Third", "Student", 2000, 5.0});
    combined = left.union_with(right);
    common = left.intersection_with(right);
    EXPECT_EQ(combined.size(), 2u);
    EXPECT_EQ(common.size(), 1u);
    ASSERT_TRUE(combined.find(original));
    ASSERT_TRUE(common.find(original));
    EXPECT_EQ(combined.find(original)->get(), original);
    EXPECT_EQ(common.find(original)->get(), original);

    left.insert(Student{4, "Fourth", "Student", 2000, 5.0});
    left.insert(Student{5, "Fifth", "Student", 2000, 5.0});
    combined = left.union_with(right);
    common = left.intersection_with(right);
    EXPECT_EQ(combined.size(), 4u);
    EXPECT_EQ(common.size(), 1u);
    ASSERT_TRUE(combined.find(original));
    ASSERT_TRUE(common.find(original));
    EXPECT_EQ(combined.find(original)->get(), original);
    EXPECT_EQ(common.find(original)->get(), original);
    EXPECT_EQ(right.find(original)->get().first_name, "Right");
}

TEST(Set, SupportsCompatibleCustomComparators) {
    Set<int, std::greater<int>> left;
    Set<int, std::greater<int>> right;
    left.insert(1);
    right.insert(2);
    right.insert(3);
    const auto combined = left.union_with(right);
    std::vector<int> visited;
    combined.for_each([&](const int& value) { visited.push_back(value); });
    EXPECT_EQ(visited, (std::vector<int>{3, 2, 1}));
    EXPECT_TRUE(left.is_subset_of(combined));
    EXPECT_EQ(combined.intersection_with(left), left);
    EXPECT_EQ(combined.difference(right), left);
}

TEST(Set, AllBinaryResultsUseLeftComparatorForEverySizeBranch) {
    using comparator_test::CountingOrder;
    using IntSet = Set<int, CountingOrder>;

    // exercise smaller, equal and larger left operands
    for (int left_size : {1, 3, 4}) {
        SCOPED_TRACE(left_size);
        int left_calls = 0;
        int right_calls = 0;
        IntSet left(CountingOrder{std::ref(left_calls)});
        IntSet right(CountingOrder{std::ref(right_calls)});
        for (int value = 1; value <= left_size; ++value) {
            left.insert(value);
        }
        for (int value : {1, 2, 3}) {
            right.insert(value);
        }

        auto combined = left.union_with(right);
        auto common = left.intersection_with(right);
        auto difference = left.difference(right);

        // make even an empty result searchable so its comparator is actually called
        for (auto& result : {std::ref(combined), std::ref(common), std::ref(difference)}) {
            result.get().insert(99);
            left_calls = 0;
            right_calls = 0;
            EXPECT_TRUE(result.get().contains(99));
            EXPECT_GT(left_calls, 0);
            EXPECT_EQ(right_calls, 0);
        }
    }
}
