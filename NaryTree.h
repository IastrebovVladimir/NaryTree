#ifndef NARY_TREE_2_H
#define NARY_TREE_2_H

#include <cstddef>
#include <stdexcept>

#include "TreeNode.h"
#include "Pointers/UniquePtr.h"
#include "Sequence/ArraySequence.h"
#include "Sequence/IEnumerator.h"
#include "Queue.h"
#include "Stack.h"


template<typename T>
class NaryTree {
private:
    using Node = TreeNode<T>;

    UniquePtr<Node> root;
    std::size_t arity;
    int length = 0;

    Node* GetNodeByPath(const MutableArraySequence<std::size_t>& path) {
        Node* node = root.Get();

        if (node == nullptr) {
            throw std::out_of_range("GetNodeByPath: tree is empty");
        }

        for (int i = 0; i < path.GetLength(); i++) {
            std::size_t childIndex = path.Get(i);

            if (childIndex >= node->childCount) {
                throw std::out_of_range("GetNodeByPath: child does not exist");
            }

            node = &node->children[childIndex];
        }

        return node;
    }

    class BFSEnumerator : public IEnumerator<T> {
    private:
        Queue<const Node*> pending;
        const Node* current = nullptr;

    public:
        explicit BFSEnumerator(const Node* root) {
            if (root != nullptr) {
                pending.Push(root);
            }
        }

        bool MoveNext() override {
            if (pending.IsEmpty()) {
                current = nullptr;
                return false;
            }

            current = pending.Front();
            pending.Pop();

            for (std::size_t i = 0; i < current->childCount; i++) {
                pending.Push(&current->children[i]);
            }

            return true;
        }

        const Node* CurrentNode() const {
            if (current == nullptr) {
                throw std::logic_error("BFSEnumerator: no current node");
            }

            return current;
        }

        const T& Current() const override {
            return CurrentNode()->value;
        }
    };

    class OrderedEnumerator : public IEnumerator<T> {
    private:
        struct Task {
            const Node* node;
            bool visit;

            Task(const Node* node, bool visit) : node(node), visit(visit) {}
        };

        Stack<Task> pending;
        const Node* current = nullptr;
        MutableArraySequence<int> order;

    public:
        OrderedEnumerator(const Node* root, const MutableArraySequence<int>& sourceOrder) {
            int orderLength = sourceOrder.GetLength();

            if (orderLength == 0) {
                throw std::invalid_argument("OrderedEnumerator: order is empty");
            }

            if (orderLength - 1 != arity) {
                throw std::invalid_argument("OrderedEnumerator: invalid order length");
            }

            bool hasRoot = false;

            for (int i = 0; i < orderLength; i++) {
                int action = sourceOrder.Get(i);

                if (action < -1) {
                    throw std::invalid_argument("OrderedEnumerator: invalid action");
                }

                if (action >= 0) {
                    std::size_t childIndex = action;

                    if (childIndex >= arity) {
                        throw std::out_of_range("OrderedEnumerator: child index out of range");
                    }
                }

                if (action == -1) {
                    hasRoot = true;
                }

                for (int j = 0; j < i; j++) {
                    if (sourceOrder.Get(j) == action) {
                        throw std::invalid_argument("OrderedEnumerator: duplicate action");
                    }
                }
            }

            if (!hasRoot) {
                throw std::invalid_argument("OrderedEnumerator: root action is missing");
            }

            for (int i = 0; i < orderLength; i++) {
                order.Append(sourceOrder.Get(i));
            }

            if (root != nullptr) {
                pending.Push(Task(root, false));
            }
        }

        OrderedEnumerator(const OrderedEnumerator&) = delete;
        OrderedEnumerator& operator=(const OrderedEnumerator&) = delete;

        bool MoveNext() override {
            current = nullptr;

            while (!pending.IsEmpty()) {
                Task task = pending.Top();
                pending.Pop();

                if (task.visit) {
                    current = task.node;
                    return true;
                }

                for (int i = order.GetLength(); i > 0; i--) {
                    int action = order.Get(i - 1);

                    if (action == -1) {
                        pending.Push(Task(task.node, true));
                    } else {
                        std::size_t childIndex = action;

                        if (childIndex < task.node->childCount) {
                            const Node* child = &task.node->children[childIndex];
                            pending.Push(Task(child, false));
                        }
                    }
                }
            }

            return false;
        }

        const T& Current() const override {
            if (current == nullptr) {
                throw std::logic_error("OrderedEnumerator: no current element");
            }

            return current->value;
        }
    };

    class KPLEnumerator : public IEnumerator<T> {
    private:
        Stack<const Node*> pending;
        const Node* current = nullptr;

    public:
        explicit KPLEnumerator(const Node* root) {
            if (root != nullptr) {
                pending.Push(root);
            }
        }

        bool MoveNext() override {
            if (pending.IsEmpty()) {
                current = nullptr;
                return false;
            }

            current = pending.Top();
            pending.Pop();

            for (std::size_t i = 0; i < current->childCount; i++) {
                pending.Push(&current->children[i]);
            }

            return true;
        }

        const T& Current() const override {
            if (current == nullptr) {
                throw std::logic_error("KPLEnumerator: no current element");
            }

            return current->value;
        }
    };

    class KLPEnumerator : public IEnumerator<T> {
    private:
        Stack<const Node*> pending;
        const Node* current = nullptr;

    public:
        explicit KLPEnumerator(const Node* root) {
            if (root != nullptr) {
                pending.Push(root);
            }
        }

        bool MoveNext() override {
            if (pending.IsEmpty()) {
                current = nullptr;
                return false;
            }

            current = pending.Top();
            pending.Pop();

            for (std::size_t i = current->childCount; i > 0; i--) {
                pending.Push(&current->children[i - 1]);
            }

            return true;
        }

        const Node* CurrentNode() const {
            if (current == nullptr) {
                throw std::logic_error("KLPEnumerator: no current node");
            }

            return current;
        }

        const T& Current() const override {
            return CurrentNode()->value;
        }
    };

    class LPKEnumerator : public IEnumerator<T> {
    private:
        struct NodeState {
            const Node* node;
            bool ready;

            NodeState(const Node* node, bool ready) : node(node), ready(ready) {}
        };

        Stack<NodeState> pending;
        const Node* current = nullptr;

    public:
        explicit LPKEnumerator(const Node* root) {
            if (root != nullptr) {
                pending.Push(NodeState(root, false));
            }
        }

        bool MoveNext() override {
            current = nullptr;

            while (!pending.IsEmpty()) {
                NodeState item = pending.Top();
                pending.Pop();

                if (item.ready) {
                    current = item.node;
                    return true;
                }

                pending.Push(NodeState(item.node, true));

                for (std::size_t i = item.node->childCount; i > 0; i--) {
                    const Node* child = &item.node->children[i - 1];
                    pending.Push(NodeState(child, false));
                }
            }

            return false;
        }

        const Node* CurrentNode() const {
            if (current == nullptr) {
                throw std::logic_error("LPKEnumerator: no current node");
            }

            return current;
        }

        const T& Current() const override {
            return CurrentNode()->value;
        }
    };

    class PLKEnumerator : public IEnumerator<T> {
    private:
        struct NodeState {
            const Node* node;
            bool ready;

            NodeState(const Node* node, bool ready) : node(node), ready(ready) {}
        };

        Stack<NodeState> pending;
        const Node* current = nullptr;

    public:
        explicit PLKEnumerator(const Node* root) {
            if (root != nullptr) {
                pending.Push(NodeState(root, false));
            }
        }

        bool MoveNext() override {
            current = nullptr;

            while (!pending.IsEmpty()) {
                NodeState item = pending.Top();
                pending.Pop();

                if (item.ready) {
                    current = item.node;
                    return true;
                }

                pending.Push(NodeState(item.node, true));

                for (std::size_t i = 0; i < item.node->childCount; i++) {
                    const Node* child = &item.node->children[i];
                    pending.Push(NodeState(child, false));
                }
            }

            return false;
        }

        const Node* CurrentNode() const {
            if (current == nullptr) {
                throw std::logic_error("PLKEnumerator: no current node");
            }

            return current;
        }

        const T& Current() const override {
            return CurrentNode()->value;
        }
    };

public:
    explicit NaryTree(std::size_t n) : root(nullptr), arity(n) {
        if (n == 0) {
            throw std::invalid_argument("NaryTree: n must be positive");
        }
    }

    NaryTree(const NaryTree&) = delete;
    NaryTree& operator=(const NaryTree&) = delete;

    Enumerator<T> GetEnumerator() const {
        return Enumerator<T>(new BFSEnumerator(root.Get()));
    }

    Enumerator<T> GetKPLEnumerator() const {
        return Enumerator<T>(new KPLEnumerator(root.Get()));
    }

    Enumerator<T> GetKLPEnumerator() const {
        return Enumerator<T>(new KLPEnumerator(root.Get()));
    }

    Enumerator<T> GetLPKEnumerator() const {
        return Enumerator<T>(new LPKEnumerator(root.Get()));
    }

    Enumerator<T> GetPLKEnumerator() const {
        return Enumerator<T>(new PLKEnumerator(root.Get()));
    }

    Enumerator<T> GetOrderedEnumerator(const MutableArraySequence<int>& order) const {
        return Enumerator<T>(new OrderedEnumerator(root.Get(), arity, order));
    }

    bool IsEmpty() const noexcept {
        return root.Get() == nullptr;
    }

    void SetRoot(const T& value) {
        if (!IsEmpty()) {
            throw std::logic_error("SetRoot: root already exists");
        }

        root.Reset(new Node(value, arity));
        length = 1;
    }

    void InsertChild(const MutableArraySequence<std::size_t>& parentPath, std::size_t childIndex, const T& value) {
        Node* parent = GetNodeByPath(parentPath);

        if (childIndex >= parent->arity) {
            throw std::out_of_range("InsertChild: child index out of range");
        }

        if (childIndex < parent->childCount) {
            throw std::logic_error("InsertChild: child already exists");
        }

        if (childIndex > parent->childCount) {
            throw std::out_of_range("InsertChild: children must be added consecutively");
        }

        parent->AddChild(value);
        length++;
    }

    T& GetRoot() {
        if (IsEmpty()) {
            throw std::out_of_range("GetRoot: tree is empty");
        }

        return root.Get()->value;
    }

    T& GetAtPath(const MutableArraySequence<std::size_t>& path) {
        return GetNodeByPath(path)->value;
    }

    int GetLength() const {
        return length;
    }

    std::size_t GetArity() const noexcept {
        return arity;
    }
};

#endif