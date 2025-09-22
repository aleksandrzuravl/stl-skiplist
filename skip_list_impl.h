#ifndef SKIP_LIST_IMPL_H
#define SKIP_LIST_IMPL_H

#include <cassert>

template <typename T, typename Compare, typename Allocator>
typename skip_list<T, Compare, Allocator>::iterator
skip_list<T, Compare, Allocator>::upper_bound(const value_type& value) {
    skip_node* current = head;
    for (size_type i = current_level; i-- > 0;) {
        assert(current != nullptr); 
        assert(current->forward[i] != nullptr); 

        while (current->forward[i] && !compare(value, current->forward[i]->value)) {
            current = current->forward[i];
        }
    }
    return iterator(current->forward[0]);
}


template <typename T, typename Compare, typename Allocator>
typename skip_list<T, Compare, Allocator>::const_iterator
skip_list<T, Compare, Allocator>::upper_bound(const value_type& value) const {
    skip_node* current = head;
    for (size_type i = current_level; i-- > 0;) {
        assert(current != nullptr);
        assert(current->forward[i] != nullptr);
        while (current->forward[i] && !compare(value, current->forward[i]->value)) {
            current = current->forward[i];
        }
    }
    return const_iterator(current->forward[0]);
}

template <typename T, typename Compare, typename Allocator>
typename skip_list<T, Compare, Allocator>::size_type
skip_list<T, Compare, Allocator>::random_level() {
    size_type level = 1;
    while (level < MAX_LEVEL && probability_distribution(random_engine) < PROBABILITY) {
        ++level;
    }
    return level;
}

template <typename T, typename Compare, typename Allocator>
typename skip_list<T, Compare, Allocator>::skip_node*
skip_list<T, Compare, Allocator>::create_node(size_type level, const value_type& value) {
    skip_node* new_node = node_allocator.allocate(1);
    std::allocator_traits<node_allocator_type>::construct(node_allocator, new_node, level, value);
    return new_node;
}

template <typename T, typename Compare, typename Allocator>
void skip_list<T, Compare, Allocator>::destroy_node(skip_node* node) {
    std::allocator_traits<node_allocator_type>::destroy(node_allocator, node);
    node_allocator.deallocate(node, 1);
}

template <typename T, typename Compare, typename Allocator>
void skip_list<T, Compare, Allocator>::destroy_nodes() noexcept {
    if (!head) return;

    skip_node* current = head->forward[0];
    while (current) {
        skip_node* next = current->forward[0];
        destroy_node(current);
        current = next;
    }
    destroy_node(head);
    head = nullptr;
}

#endif
