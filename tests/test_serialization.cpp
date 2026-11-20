#include "models/student_streams.hpp"
#include "set/set_streams.hpp"
#include "tree/tree_streams.hpp"
#include "tests_utils/avl_tree_utils.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

TEST(Serialization, PreservesIntegerTreeShapeAndInvariants) {
    AVLTree<int> tree;
    avl_test::TreeChecks::insert_values(tree, {8, 4, 12});
    EXPECT_EQ(tree.serialize(), "8 4 # # 12 # #");
    auto copy = AVLTree<int>::deserialize(tree.serialize());
    avl_test::TreeChecks::expect_valid(copy, 3);
    EXPECT_EQ(copy.serialize(), tree.serialize());
    EXPECT_TRUE(copy.erase(4));
    avl_test::TreeChecks::expect_valid(copy, 2);
    EXPECT_TRUE(tree.contains(4));

    auto empty = AVLTree<int>::deserialize("  # \n");
    avl_test::TreeChecks::expect_valid(empty, 0);
    EXPECT_EQ(empty.serialize(), "#");
    auto single = AVLTree<int>::deserialize("9 # #");
    avl_test::TreeChecks::expect_valid(single, 1);
}

TEST(Serialization, PreservesStringsWithSpecialCharacters) {
    AVLTree<std::string> tree;
    for (const std::string value : {"", "#", "hello world", "a\"b\\c", "line\nbreak"}) {
        tree.insert(value);
    }
    auto copy = AVLTree<std::string>::deserialize(tree.serialize());
    EXPECT_EQ(copy.size(), tree.size());
    EXPECT_EQ(copy.serialize(), tree.serialize());
    tree.for_each(TraversalOrder::LKP, [&](const std::string& value) {
        EXPECT_TRUE(copy.contains(value));
    });
}

TEST(Serialization, RejectsMalformedAndInvalidTrees) {
    for (const std::string data : {
            "", "8 #", "# #", "#abc", "8abc # #", "8 # # extra",
            "8 4 # 10 # # #", "8 8 # # #", "1 # 2 # 3 # #", "999999999999999999999 # #"}) {
        SCOPED_TRACE(data);
        EXPECT_THROW(AVLTree<int>::deserialize(data), std::invalid_argument);
    }
    AVLTree<int> destination;
    avl_test::TreeChecks::insert_values(destination, {7});
    EXPECT_THROW(destination = AVLTree<int>::deserialize("8 #"), std::invalid_argument);
    avl_test::TreeChecks::expect_valid(destination, 1);
    EXPECT_TRUE(destination.contains(7));
    for (const std::string data : {"unquoted # #", "\"unfinished", "\"ok\"junk # #"}) {
        EXPECT_THROW(AVLTree<std::string>::deserialize(data), std::invalid_argument);
    }

    // an unfinished long chain must be rejected before it can exhaust the call stack
    std::string deep;
    for (int value = 0; value < 1000; ++value) {
        deep += std::to_string(value) + " # ";
    }
    EXPECT_THROW(AVLTree<int>::deserialize(deep), std::invalid_argument);
}

TEST(Serialization, PreservesFloatingPointPrecisionAndRejectsNonFiniteValues) {
    AVLTree<double> tree;
    tree.insert(std::nextafter(1.0, 2.0));
    tree.insert(0.1);
    auto copy = AVLTree<double>::deserialize(tree.serialize());
    EXPECT_EQ(copy.size(), 2u);
    EXPECT_TRUE(copy.contains(std::nextafter(1.0, 2.0)));
    EXPECT_TRUE(copy.contains(0.1));
    tree.insert(std::numeric_limits<double>::infinity());
    EXPECT_THROW(tree.serialize(), std::invalid_argument);
    EXPECT_THROW(AVLTree<double>::deserialize("nan # #"), std::invalid_argument);
    EXPECT_THROW(AVLTree<double>::deserialize("inf # #"), std::invalid_argument);
}

TEST(Serialization, UsesProvidedComparatorState) {
    using Tree = AVLTree<int, avl_test::CopyCheckedOrder>;
    Tree tree(avl_test::CopyCheckedOrder{true});
    for (int value : {2, 1, 3}) {
        tree.insert(value);
    }
    auto copy = Tree::deserialize(tree.serialize(), avl_test::CopyCheckedOrder{true});
    EXPECT_EQ(copy.serialize(), "2 3 # # 1 # #");
    EXPECT_TRUE(copy.contains(1));
    EXPECT_THROW(Tree::deserialize(tree.serialize(), avl_test::CopyCheckedOrder{false}),
                 std::invalid_argument);
}

TEST(Student, StreamRoundTripAndFailedReadPreserveDestination) {
    const Student original{17, "Ivan \"I\"", "Petrov \\ #", 2005, std::nextafter(4.0, 5.0)};
    std::stringstream stream;
    stream << original;
    Student restored;
    stream >> restored;
    ASSERT_FALSE(stream.fail());
    EXPECT_EQ(restored, original);

    for (const std::string record : {
            "0 \"Ivan\" \"Petrov\" 2005 4.5",
            "17 Ivan \"Petrov\" 2005 4.5",
            "17 \"Ivan\" \"Petrov\" 2005abc 4.5",
            "17 \"Ivan\" \"Petrov\" 2005",
            "17 \"Ivan\" \"Petrov\" 2005 nan",
            "17 \"Ivan\" \"Petrov\" 2005 4.5junk"}) {
        std::istringstream input(record);
        input >> restored;
        EXPECT_TRUE(input.fail()) << record;
        EXPECT_EQ(restored, original);
    }
}

TEST(Serialization, PreservesCompleteStudentRecords) {
    AVLTree<Student> tree;
    const Student first{2, "Anna Maria", "Smith", 2004, 4.7};
    const Student second{1, "Ivan", "# \"Petrov\"", 2005, 3.5};
    tree.insert(first);
    tree.insert(second);
    EXPECT_FALSE(tree.insert(Student{2, "Other", "Student", 2000, 2.0}));

    auto copy = AVLTree<Student>::deserialize(tree.serialize());
    EXPECT_EQ(copy.size(), 2u);
    const auto found_first = copy.find(first);
    const auto found_second = copy.find(second);
    ASSERT_TRUE(found_first && found_second);
    EXPECT_EQ(found_first->get(), first);
    EXPECT_EQ(found_second->get(), second);
    EXPECT_EQ(copy.serialize(), tree.serialize());

    std::ostringstream output;
    output << Student{};
    EXPECT_TRUE(output.fail());
    EXPECT_THROW(AVLTree<Student>::deserialize("17 \"Ivan\" \"Petrov\" 2005 # #"),
                 std::invalid_argument);
}

TEST(AvlTree, StreamOutputShowsOrderedValuesAndEmptyTree) {
    AVLTree<int> tree;
    std::ostringstream output;
    output << tree;
    EXPECT_EQ(output.str(), "[]");

    for (int value : {2, 3, 1}) {
        tree.insert(value);
    }
    output.str("");
    output << tree;
    EXPECT_EQ(output.str(), "[1, 2, 3]");
}

TEST(Set, StreamOutputShowsComparatorOrderAndEmptySet) {
    Set<int, std::greater<int>> set;
    std::ostringstream output;
    output << set;
    EXPECT_EQ(output.str(), "{}");

    for (int value : {2, 3, 1}) {
        set.insert(value);
    }
    output.str("");
    output << set;
    EXPECT_EQ(output.str(), "{3, 2, 1}");
}
