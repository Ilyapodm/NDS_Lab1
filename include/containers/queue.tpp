#include <stdexcept>

template <typename T>
Queue<T>::~Queue() {
    clear();
}

template <typename T>
Queue<T>::Queue(const Queue& other) {
    // the temporary clears partial copies iteratively if copying throws
    Queue copy;
    std::optional<std::reference_wrapper<const Node>> current;
    if (other.head_) {
        current = std::cref(*other.head_);
    }
    while (current) {
        const Node& node = current->get();
        copy.push(node.value);
        if (node.next) {
            current = std::cref(*node.next);
        } else {
            current.reset();
        }
    }
    *this = std::move(copy);
}

template <typename T>
Queue<T>& Queue<T>::operator=(const Queue& other) {
    if (this != &other) {
        Queue copy(other);
        // commit only after all values have been copied successfully
        *this = std::move(copy);
    }
    return *this;
}

template <typename T>
Queue<T>::Queue(Queue&& other) noexcept
    : head_(std::move(other.head_)),
      tail_(std::exchange(other.tail_, std::nullopt)),
      size_(std::exchange(other.size_, 0)) {}

template <typename T>
Queue<T>& Queue<T>::operator=(Queue&& other) noexcept {
    if (this != &other) {
        clear();
        head_ = std::move(other.head_);
        tail_ = std::exchange(other.tail_, std::nullopt);
        size_ = std::exchange(other.size_, 0);
    }
    return *this;
}

template <typename T>
void Queue<T>::push(T value) {
    // finish allocation and value construction before changing the queue
    auto node = std::make_unique<Node>(std::move(value));
    auto new_tail = std::ref(*node);

    if (tail_) {
        tail_->get().next = std::move(node);
    } else {
        head_ = std::move(node);
    }
    tail_ = new_tail;
    ++size_;
}

template <typename T>
void Queue<T>::pop() {
    if (empty()) {
        throw std::out_of_range("pop from an empty queue");
    }

    // keep the old head alive while moving its next link
    auto removed = std::move(head_);
    head_ = std::move(removed->next);
    --size_;
    if (empty()) {
        tail_.reset();
    }
}

template <typename T>
T& Queue<T>::front() {
    if (empty()) {
        throw std::out_of_range("front of an empty queue");
    }
    return head_->value;
}

template <typename T>
const T& Queue<T>::front() const {
    if (empty()) {
        throw std::out_of_range("front of an empty queue");
    }
    return head_->value;
}

template <typename T>
std::size_t Queue<T>::size() const noexcept {
    return size_;
}

template <typename T>
bool Queue<T>::empty() const noexcept {
    return size_ == 0;
}

template <typename T>
void Queue<T>::clear() noexcept {
    // destroy one node at a time to avoid recursion through a long chain
    tail_.reset();
    while (head_) {
        auto removed = std::move(head_);
        head_ = std::move(removed->next);
    }
    size_ = 0;
}
