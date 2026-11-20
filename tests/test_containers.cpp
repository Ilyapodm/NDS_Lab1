#include "tests_utils/queue_utils.hpp"
#include "containers/queue.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

TEST(Queue, EmptyQueueRejectsFrontAndPop) {
    Queue<int> queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0u);
    EXPECT_THROW(queue.front(), std::out_of_range);
    const auto& read_only = queue;
    EXPECT_THROW(read_only.front(), std::out_of_range);
    EXPECT_THROW(queue.pop(), std::out_of_range);
    EXPECT_TRUE(queue.empty());
}

TEST(Queue, PushAndPopPreserveFifoOrder) {
    Queue<int> queue;
    queue.push(1);
    EXPECT_EQ(queue.size(), 1u);
    EXPECT_FALSE(queue.empty());
    queue.push(2);
    EXPECT_EQ(queue.size(), 2u);
    EXPECT_EQ(queue.front(), 1);

    queue.pop();
    EXPECT_EQ(queue.size(), 1u);
    queue.push(3);
    EXPECT_EQ(queue.size(), 2u);
    EXPECT_EQ(queue.front(), 2);
    queue.front() = 20;
    const auto& read_only = queue;
    EXPECT_EQ(read_only.front(), 20);

    queue.pop();
    EXPECT_EQ(queue.size(), 1u);
    EXPECT_EQ(queue.front(), 3);
    queue.pop();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0u);

    // draining the queue must reset the tail before the next push
    queue.push(4);
    EXPECT_EQ(queue.front(), 4);
    EXPECT_EQ(queue.size(), 1u);
}

TEST(Queue, ClearHandlesLongChainAndAllowsReuse) {
    Queue<int> queue;
    for (int value = 0; value < 100000; ++value) {
        queue.push(value);
    }
    EXPECT_EQ(queue.size(), 100000u);
    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0u);
    queue.clear();
    EXPECT_TRUE(queue.empty());
    queue.push(7);
    EXPECT_EQ(queue.front(), 7);
    EXPECT_EQ(queue.size(), 1u);
}

TEST(Queue, MoveTransfersContentsAndAllowsReuse) {
    Queue<int> source;
    source.push(1);
    source.push(2);
    Queue<int> moved(std::move(source));
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(source.size(), 0u);
    EXPECT_EQ(moved.size(), 2u);
    EXPECT_EQ(moved.front(), 1);

    source.push(9);
    source = std::move(moved);
    EXPECT_TRUE(moved.empty());
    EXPECT_EQ(moved.size(), 0u);
    EXPECT_EQ(source.size(), 2u);
    EXPECT_EQ(source.front(), 1);
    source.pop();
    EXPECT_EQ(source.front(), 2);
    EXPECT_EQ(source.size(), 1u);
    moved.push(3);
    EXPECT_EQ(moved.front(), 3);

    auto& same_queue = source;
    source = std::move(same_queue);
    EXPECT_EQ(source.size(), 1u);
    EXPECT_EQ(source.front(), 2);
}

TEST(Queue, SupportsMoveOnlyValuesAndNodeReferences) {
    Queue<std::unique_ptr<int>> values;
    values.push(std::make_unique<int>(42));
    EXPECT_EQ(*values.front(), 42);
    values.pop();
    EXPECT_TRUE(values.empty());

    // bfs can queue references without copying the nodes
    const int value = 7;
    Queue<std::reference_wrapper<const int>> references;
    references.push(std::cref(value));
    EXPECT_EQ(&references.front().get(), &value);
    references.pop();
    EXPECT_TRUE(references.empty());
}

TEST(Queue, CopyPreservesOrderAndCreatesIndependentNodes) {
    Queue<int> source;
    source.push(1);
    source.push(2);

    Queue<int> copy(source);
    EXPECT_EQ(copy.size(), 2u);
    EXPECT_NE(&copy.front(), &source.front());
    copy.front() = 7;
    EXPECT_EQ(source.front(), 1);
    copy.pop();
    EXPECT_EQ(copy.front(), 2);
    copy.push(3);
    EXPECT_EQ(source.size(), 2u);

    Queue<int> assigned;
    assigned.push(9);
    assigned = source;
    EXPECT_EQ(assigned.size(), 2u);
    EXPECT_NE(&assigned.front(), &source.front());
    EXPECT_EQ(assigned.front(), 1);
    assigned.pop();
    EXPECT_EQ(assigned.front(), 2);
    EXPECT_EQ(source.front(), 1);

    const auto& same_queue = source;
    source = same_queue;
    EXPECT_EQ(source.size(), 2u);
    EXPECT_EQ(source.front(), 1);

    Queue<int> empty;
    Queue<int> empty_copy(empty);
    EXPECT_TRUE(empty_copy.empty());
    assigned = empty;
    EXPECT_TRUE(assigned.empty());
    EXPECT_EQ(assigned.size(), 0u);
    assigned.push(4);
    EXPECT_EQ(assigned.front(), 4);
}

TEST(Queue, FailedCopyLeavesSourceAndTargetIntact) {
    using queue_test::CopyCheckedValue;
    Queue<CopyCheckedValue> source;
    source.push(CopyCheckedValue{1});
    source.push(CopyCheckedValue{2});
    Queue<CopyCheckedValue> target;
    target.push(CopyCheckedValue{9});

    // allow one node to be copied before triggering the exception
    CopyCheckedValue::copies_before_failure = 1;
    EXPECT_THROW((Queue<CopyCheckedValue>(source)), std::runtime_error);
    CopyCheckedValue::copies_before_failure = 1;
    EXPECT_THROW(target = source, std::runtime_error);
    CopyCheckedValue::copies_before_failure = -1;

    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target.front().value, 9);
    EXPECT_EQ(source.size(), 2u);
    EXPECT_EQ(source.front().value, 1);
    source.pop();
    EXPECT_EQ(source.front().value, 2);

    target = source;
    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target.front().value, 2);
}
