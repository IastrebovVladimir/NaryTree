#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <stdexcept>

template<typename T>
class Queue {
private:
    struct QueueItem {
        T data;
        QueueItem* next;

        explicit QueueItem(const T& value) : data(value), next(nullptr) {}
    };

    QueueItem* head;
    QueueItem* tail;
    std::size_t length;

public:
    Queue() noexcept : head(nullptr), tail(nullptr), length(0) {}

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&& other) noexcept : head(other.head), tail(other.tail), length(other.length) {
        other.head = nullptr;
        other.tail = nullptr;
        other.length = 0;
    }

    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            Clear();

            head = other.head;
            tail = other.tail;
            length = other.length;

            other.head = nullptr;
            other.tail = nullptr;
            other.length = 0;
        }

        return *this;
    }

    bool IsEmpty() const noexcept {
        return head == nullptr;
    }

    std::size_t GetLength() const noexcept {
        return length;
    }

    void Push(const T& item) {
        QueueItem* newItem = new QueueItem(item);

        if (tail == nullptr) {
            head = newItem;
        } else {
            tail->next = newItem;
        }

        tail = newItem;
        length++;
    }

    const T& Front() const {
        if (IsEmpty()) {
            throw std::out_of_range("Queue::Front: queue is empty");
        }

        return head->data;
    }

    void Pop() {
        if (IsEmpty()) {
            throw std::out_of_range("Queue::Pop: queue is empty");
        }

        QueueItem* removed = head;
        head = head->next;

        if (head == nullptr) {
            tail = nullptr;
        }

        delete removed;
        length--;
    }

    void Clear() noexcept {
        while (head != nullptr) {
            QueueItem* removed = head;
            head = head->next;
            delete removed;
        }

        tail = nullptr;
        length = 0;
    }

    ~Queue() noexcept {
        Clear();
    }
};

#endif