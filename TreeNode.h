#ifndef TREE_NODE_2_H
#define TREE_NODE_2_H

#include <cstddef>
#include <stdexcept>

#include "Pointers/UniquePtr.h"

template<typename T>
struct TreeNode {
    T value;
    std::size_t arity;
    std::size_t childCount;

    UniquePtr<TreeNode<T>[]> children;

    TreeNode() : value{}, arity(0), childCount(0), children(nullptr) {}

    TreeNode(const T& value, std::size_t arity) : value(value), arity(arity), childCount(0), children(nullptr) {
        if (arity == 0) {
            throw std::invalid_argument("TreeNode: arity must be greater than zero");
        }
    }

    TreeNode& AddChild(const T& childValue) {
        if (childCount >= arity) {
            throw std::out_of_range("TreeNode: no space for another child");
        }

        if (!children.Get()) {
            children.Reset(new TreeNode<T>[arity]);
        }

        TreeNode<T>& child = children[childCount];

        child.value = childValue;
        child.arity = arity;

        childCount++;
        return child;
    }

    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;
};

#endif