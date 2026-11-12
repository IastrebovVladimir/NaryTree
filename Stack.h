#ifndef STACK_H
#define STACK_H

#include <cstddef>
#include <stdexcept>

template<typename T>
class Stack {
private:
    struct StackItem {
        T data;
        StackItem* next;

        explicit StackItem(const T& value) : data(value), next(nullptr) {}
    };

    StackItem* head;
    std::size_t length;

public:
    Stack() noexcept : head(nullptr), length(0) {}

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    Stack(Stack&& other) noexcept : head(other.head), length(other.length) {
        other.head = nullptr;
        other.length = 0;
    }

    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            Clear();

            head = other.head;
            length = other.length;

            other.head = nullptr;
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
        StackItem* newItem = new StackItem(item);

        newItem->next = head;
        head = newItem;

        length++;
    }

    const T& Top() const {
        if (IsEmpty()) {
            throw std::out_of_range("Stack::Top: stack is empty");
        }

        return head->data;
    }

    void Pop() {
        if (IsEmpty()) {
            throw std::out_of_range("Stack::Pop: stack is empty");
        }

        StackItem* removed = head;
        head = head->next;

        delete removed;
        length--;
    }

    void Clear() noexcept {
        while (head != nullptr) {
            StackItem* removed = head;
            head = head->next;
            delete removed;
        }

        length = 0;
    }

    ~Stack() noexcept {
        Clear();
    }
};

#endif
