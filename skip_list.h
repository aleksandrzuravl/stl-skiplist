#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <random>
#include <vector>

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
class skip_list {
public:
    using value_type = T;
    using value_compare = Compare;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

    struct skip_node {
        value_type value;
        std::vector<skip_node*> forward;

        template <typename... Args>
        skip_node(size_type levels, Args&&... args)
            : value(std::forward<Args>(args)...), forward(levels, nullptr) {}
    };

    using node_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<skip_node>;

    class iterator {
        skip_node* current;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename skip_list::value_type;
        using difference_type = typename skip_list::difference_type;
        using reference = value_type&;
        using pointer = value_type*;

        iterator() noexcept : current(nullptr) {}
        explicit iterator(skip_node* node) noexcept : current(node) {}

        reference operator*() const noexcept { return current->value; }
        pointer operator->() const noexcept { return &current->value; }

        iterator& operator++() noexcept {
            current = current->forward[0];
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const noexcept { return current == other.current; }
        bool operator!=(const iterator& other) const noexcept { return !(*this == other); }
    };

    class const_iterator {
        const skip_node* current;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename skip_list::value_type;
        using difference_type = typename skip_list::difference_type;
        using reference = const value_type&;
        using pointer = const value_type*;

        const_iterator() noexcept : current(nullptr) {}
        explicit const_iterator(const skip_node* node) noexcept : current(node) {}
        const_iterator(const iterator& it) noexcept : current(it.current) {}

        reference operator*() const noexcept { return current->value; }
        pointer operator->() const noexcept { return &current->value; }

        const_iterator& operator++() noexcept {
            current = current->forward[0];
            return *this;
        }

        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const noexcept { return current == other.current; }
        bool operator!=(const const_iterator& other) const noexcept { return !(*this == other); }
    };

    explicit skip_list(const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : node_allocator(alloc), compare(comp), head(nullptr), current_level(1), element_count(0),
          random_engine(std::random_device{}()), probability_distribution(0.0f, 1.0f) {
        init_head();
    }

    skip_list(const skip_list& other) : skip_list(other.compare, other.node_allocator) {
        for (const auto& v : other) {
            insert(v);
        }
    }

    skip_list(skip_list&& other) noexcept
        : node_allocator(std::move(other.node_allocator)),
          compare(std::move(other.compare)),
          head(std::exchange(other.head, nullptr)),
          current_level(std::exchange(other.current_level, 1)),
          element_count(std::exchange(other.element_count, 0)),
          random_engine(std::move(other.random_engine)),
          probability_distribution(std::move(other.probability_distribution)) {}

    skip_list(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : skip_list(comp, alloc) {
        for (const auto& value : init) {
            insert(value);
        }
    }

    ~skip_list() {
        destroy_nodes();
    }

    skip_list& operator=(const skip_list& other) {
        if (this != &other) {
            clear();
            compare = other.compare;
            for (const auto& v : other) {
                insert(v);
            }
        }
        return *this;
    }

    skip_list& operator=(skip_list&& other) noexcept {
        if (this != &other) {
            destroy_nodes();
            node_allocator = std::move(other.node_allocator);
            compare = std::move(other.compare);
            head = std::exchange(other.head, nullptr);
            current_level = std::exchange(other.current_level, 1);
            element_count = std::exchange(other.element_count, 0);
            random_engine = std::move(other.random_engine);
            probability_distribution = std::move(other.probability_distribution);
        }
        return *this;
    }

    skip_list& operator=(std::initializer_list<value_type> init) {
        clear();
        for (const auto& value : init) {
            insert(value);
        }
        return *this;
    }

    iterator begin() noexcept { return iterator(head->forward[0]); }
    const_iterator begin() const noexcept { return const_iterator(head->forward[0]); }
    const_iterator cbegin() const noexcept { return begin(); }
    iterator end() noexcept { return iterator(nullptr); }
    const_iterator end() const noexcept { return const_iterator(nullptr); }
    const_iterator cend() const noexcept { return end(); }

    bool empty() const noexcept { return element_count == 0; }
    size_type size() const noexcept { return element_count; }
    size_type max_size() const noexcept { return node_allocator.max_size(); }
    allocator_type get_allocator() const noexcept { return allocator_type(node_allocator); }

    void clear() noexcept {
        destroy_nodes();
        init_head();
        element_count = 0;
        current_level = 1;
    }

    std::pair<iterator, bool> insert(const value_type& value) {
    std::vector<skip_node*> update(MAX_LEVEL, nullptr);
    skip_node* current = head;

    for (size_type i = current_level; i-- > 0;) {
        while (current->forward[i] && compare(current->forward[i]->value, value)) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if (current && !compare(value, current->value) && !compare(current->value, value)) {
        return {iterator(current), false};
    }

    size_type new_level = random_level();
    if (new_level > current_level) {
        for (size_type i = current_level; i < new_level; ++i) {
            update[i] = head;
        }
        current_level = new_level;
    }

    skip_node* new_node = create_node(new_level, value);
    for (size_type i = 0; i < new_level; ++i) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    ++element_count;
    return {iterator(new_node), true};
}


    size_type erase(const value_type& value) {
        std::vector<skip_node*> update(MAX_LEVEL, nullptr);
        skip_node* current = head;

        for (size_type i = current_level; i-- > 0;) {
            while (current->forward[i] && compare(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        current = current->forward[0];

        if (!current || compare(value, current->value) || compare(current->value, value)) {
            return 0;
        }

        for (size_type i = 0; i < current_level; ++i) {
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }

        destroy_node(current);
        --element_count;

        while (current_level > 1 && !head->forward[current_level - 1]) {
            --current_level;
        }

        return 1;
    }

    iterator find(const value_type& value) {
        skip_node* current = head;
        for (size_type i = current_level; i-- > 0;) {
            while (current->forward[i] && compare(current->forward[i]->value, value)) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        if (current && !compare(value, current->value) && !compare(current->value, value)) {
            return iterator(current);
        }
        return end();
    }

private:
    static constexpr size_type MAX_LEVEL = 32;
    static constexpr float PROBABILITY = 0.5f;

    node_allocator_type node_allocator;
    value_compare compare;
    skip_node* head;
    size_type current_level;
    size_type element_count;
    std::mt19937 random_engine;
    std::uniform_real_distribution<float> probability_distribution;

    void init_head() {
        if (!head) {
            head = create_node(MAX_LEVEL, value_type{});
            for (size_type i = 0; i < MAX_LEVEL; ++i) {
                head->forward[i] = nullptr;
            }
        }
    }

    size_type random_level() {
        size_type level = 1;
        while (level < MAX_LEVEL && probability_distribution(random_engine) < PROBABILITY) {
            ++level;
        }
        return level;
    }

    template <typename... Args>
    skip_node* create_node(size_type level, Args&&... args) {
        skip_node* new_node = node_allocator.allocate(1);
        try {
            std::allocator_traits<node_allocator_type>::construct(
                node_allocator, new_node, level, std::forward<Args>(args)...);
        } catch (...) {
            node_allocator.deallocate(new_node, 1);
            throw;
        }
        return new_node;
    }

    void destroy_node(skip_node* node) noexcept {
        if (node) {
            std::allocator_traits<node_allocator_type>::destroy(node_allocator, node);
            node_allocator.deallocate(node, 1);
        }
    }

    void destroy_nodes() noexcept {
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
};

template <typename T, typename Compare, typename Allocator>
void swap(skip_list<T, Compare, Allocator>& lhs, skip_list<T, Compare, Allocator>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif
