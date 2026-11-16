#pragma once

#include <memory>
#include <utility>

template <typename T>
struct Node{
    T value;
    std::unique_ptr<Node> left{nullptr}; 
    std::unique_ptr<Node> right{nullptr};

    std::size_t height = 1;

    explicit Node(T val) : value{std::move(val)} {}
};