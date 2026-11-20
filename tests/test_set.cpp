#include "tests_utils/comparator_utils.hpp"
#include "set/set.hpp"
#include "models/student_streams.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
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

TEST(Set, MapTransformsValuesWithoutChangingSource) {
    Set<int> source;
    for (int value : {1, 2, 3}) {
        source.insert(value);
    }
    auto result = source.map([](const int& value) { return -value; });
    EXPECT_EQ(result.size(), 3u);
    for (int value : {1, 2, 3}) {
        EXPECT_TRUE(result.contains(-value));
        EXPECT_TRUE(source.contains(value));
    }
    EXPECT_TRUE(result.erase(-1));
    EXPECT_TRUE(source.contains(1));
    EXPECT_EQ(source.size(), 3u);
}

TEST(Set, MapCollisionsKeepFirstOriginalKeyInComparatorOrder) {
    Set<Student> source;
    source.insert(Student{3, "Third", "Student", 2005, 3.0});
    source.insert(Student{1, "First", "Student", 2005, 4.0});
    source.insert(Student{2, "Second", "Student", 2005, 5.0});

    auto merge_keys = [](const Student& student) {
        Student result = student;
        result.id = 10;
        return result;
    };
    auto result = source.map(merge_keys);
    EXPECT_EQ(result.size(), 1u);
    const Student expected{10, "First", "Student", 2005, 4.0};
    const auto found = result.find(expected);
    ASSERT_TRUE(found);
    EXPECT_EQ(found->get(), expected);
    EXPECT_EQ(source.size(), 3u);

    // with descending order, the first original key is the numerically largest
    auto descending = [](const Student& left, const Student& right) { return right < left; };
    Set<Student, decltype(descending)> reversed(descending);
    source.for_each([&](const Student& student) { reversed.insert(student); });
    const auto reversed_result = reversed.map(merge_keys);
    const auto reversed_found = reversed_result.find(expected);
    ASSERT_TRUE(reversed_found);
    EXPECT_EQ(reversed_result.size(), 1u);
    EXPECT_EQ(reversed_found->get(), (Student{10, "Third", "Student", 2005, 3.0}));
}

TEST(Set, WhereSelectsSomeAllOrNoValues) {
    Set<int> source;
    for (int value : {1, 2, 3, 4}) {
        source.insert(value);
    }
    auto even = source.where([](const int& value) { return value % 2 == 0; });
    EXPECT_EQ(even.size(), 2u);
    EXPECT_TRUE(even.contains(2));
    EXPECT_TRUE(even.contains(4));
    EXPECT_EQ(source.where([](const int&) { return true; }), source);
    EXPECT_TRUE(source.where([](const int&) { return false; }).empty());
    EXPECT_TRUE(even.erase(2));
    EXPECT_TRUE(source.contains(2));
    EXPECT_EQ(source.size(), 4u);
}

TEST(Set, ReduceUsesInitialValueAndComparatorOrder) {
    Set<int, std::greater<int>> source;
    for (int value : {2, 1, 3}) {
        source.insert(value);
    }
    EXPECT_EQ(source.reduce(std::string{}, [](std::string text, const int& value) {
        return text + std::to_string(value);
    }), "321");

    // transformed and filtered sets retain the original comparator
    const auto mapped = source.map([](const int& value) { return value + 10; });
    const auto filtered = source.where([](const int& value) { return value > 1; });
    auto join = [](std::string text, const int& value) {
        return text + std::to_string(value) + " ";
    };
    EXPECT_EQ(mapped.reduce(std::string{}, join), "13 12 11 ");
    EXPECT_EQ(filtered.reduce(std::string{}, join), "3 2 ");

    auto total = source.reduce(std::make_unique<int>(10),
        [](std::unique_ptr<int> accumulator, const int& value) {
            *accumulator += value;
            return accumulator;
        });
    ASSERT_TRUE(total);
    EXPECT_EQ(*total, 16);
}

TEST(Set, FunctionalOperationsHandleEmptySetWithoutCallingHandlers) {
    Set<int> empty;
    int calls = 0;
    EXPECT_TRUE(empty.map([&](const int& value) {
        ++calls;
        return value;
    }).empty());
    EXPECT_TRUE(empty.where([&](const int&) {
        ++calls;
        return true;
    }).empty());
    EXPECT_EQ(empty.reduce(7, [&](int total, const int& value) {
        ++calls;
        return total + value;
    }), 7);
    EXPECT_EQ(calls, 0);
}

TEST(Set, FunctionalOperationsPropagateErrorsAndPreserveSource) {
    Set<int> source;
    source.insert(1);
    source.insert(2);
    auto mapper = [](const int& value) {
        if (value == 2) {
            throw std::runtime_error("mapper failed");
        }
        return value;
    };
    auto predicate = [](const int& value) {
        if (value == 2) {
            throw std::runtime_error("predicate failed");
        }
        return true;
    };
    EXPECT_THROW(source.map(mapper), std::runtime_error);
    EXPECT_THROW(source.where(predicate), std::runtime_error);
    EXPECT_THROW(source.reduce(0, [](int total, const int& value) {
        if (value == 2) {
            throw std::runtime_error("reducer failed");
        }
        return total + value;
    }), std::runtime_error);
    EXPECT_EQ(source.size(), 2u);
    EXPECT_TRUE(source.contains(1));
    EXPECT_TRUE(source.contains(2));
}

TEST(Set, SerializationRestoresContentsAndEmptySet) {
    Set<int> source;
    for (int value : {2, 1, 3}) {
        source.insert(value);
    }
    EXPECT_EQ(source.serialize(), "2 1 # # 3 # #");
    auto restored = Set<int>::deserialize(source.serialize());
    EXPECT_EQ(restored, source);
    EXPECT_EQ(restored.serialize(), source.serialize());
    EXPECT_TRUE(restored.erase(1));
    EXPECT_TRUE(source.contains(1));

    EXPECT_EQ(Set<int>{}.serialize(), "#");
    EXPECT_TRUE(Set<int>::deserialize(" # \n").empty());
    const auto single = Set<int>::deserialize("7 # #");
    EXPECT_EQ(single.size(), 1u);
    EXPECT_TRUE(single.contains(7));
}

TEST(Set, SerializationPreservesSpecialStringsAndStudentFields) {
    Set<std::string> strings;
    for (const std::string value : {"", "#", "hello world", "a\"b\\c"}) {
        strings.insert(value);
    }
    const auto restored_strings = Set<std::string>::deserialize(strings.serialize());
    EXPECT_EQ(restored_strings, strings);

    const Student original{7, "Anna Maria", "# \"Smith\"", 2005, 4.75};
    Set<Student> students;
    students.insert(original);
    const auto restored_students = Set<Student>::deserialize(students.serialize());
    EXPECT_EQ(restored_students.size(), 1u);
    const auto found = restored_students.find(original);
    ASSERT_TRUE(found);
    // set equality compares keys, so check the full student record explicitly
    EXPECT_EQ(found->get(), original);
}

TEST(Set, DeserializationRejectsInvalidDataAndPreservesAssignmentTarget) {
    Set<int> target;
    target.insert(9);
    for (const std::string data : {
            "", "2 #", "# #", "2abc # #", "2 2 # # #",
            "2 3 # # #", "1 # 2 # 3 # #"}) {
        SCOPED_TRACE(data);
        EXPECT_THROW(target = Set<int>::deserialize(data), std::invalid_argument);
        EXPECT_EQ(target.size(), 1u);
        EXPECT_TRUE(target.contains(9));
    }
}

TEST(Set, DeserializationUsesProvidedComparator) {
    using comparator_test::CountingOrder;
    using IntSet = Set<int, CountingOrder>;
    int calls = 0;
    auto restored = IntSet::deserialize("2 1 # # 3 # #", CountingOrder{std::ref(calls)});
    calls = 0;
    EXPECT_TRUE(restored.contains(1));
    EXPECT_GT(calls, 0);

    using DescendingSet = Set<int, std::greater<int>>;
    const auto descending = DescendingSet::deserialize("2 3 # # 1 # #");
    EXPECT_EQ(descending.size(), 3u);
    EXPECT_TRUE(descending.contains(3));
    EXPECT_THROW(Set<int>::deserialize(descending.serialize()), std::invalid_argument);
}
