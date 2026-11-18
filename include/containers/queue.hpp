#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

template <typename T>
class Queue {
public:
    Queue() = default;
    ~Queue();

    Queue(const Queue& other);
    Queue& operator=(const Queue& other);
    Queue(Queue&& other) noexcept;
    Queue& operator=(Queue&& other) noexcept;

    void push(T value);
    void pop();

    T& front();
    const T& front() const;

    std::size_t size() const noexcept;
    bool empty() const noexcept;
    void clear() noexcept;

private:
    struct Node {
        T value;
        std::unique_ptr<Node> next;

        explicit Node(T value) : value(std::move(value)) {}
    };

    std::unique_ptr<Node> head_;
    // the tail refers to a node owned by the chain starting at head
    std::optional<std::reference_wrapper<Node>> tail_;
    std::size_t size_ = 0;
};

#include "queue.tpp"
