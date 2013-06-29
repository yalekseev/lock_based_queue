#ifndef QUEUE_H
#define QUEUE_H

#include <memory>
#include <mutex>

template <typename T>
class queue {
private:
    struct node {
        node() : m_next(0) { }

        explicit node(const T & data) : m_data(new T(data)), m_next(0) { }
        node(const node & other) = delete;
        node & operator=(const node & other) = delete;

        std::shared_ptr<T> m_data;
        node * m_next;
    };

public:
    queue() : m_head(new node), m_tail(m_head) { }
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;


    ~queue() {
        while (m_head) {
            node * old_head = m_head;
            m_head = old_head->m_next;
            delete old_head;
        }
    }

    bool try_pop(T & val) {
        node * old_head = pop_head();
        if (0 == old_head) {
            return false;
        }

        val = *(old_head->m_data);
        delete old_head;

        return true;
    }

    void push(const T & val) {
        std::shared_ptr<T> new_data(new T(val));
        node * new_node = new node;

        std::lock_guard<std::mutex> lock(m_tail_mutex);
        m_tail->m_data = new_data;
        m_tail->m_next = new_node;
        m_tail = new_node;
    }

private:
    node * get_tail() {
        std::lock_guard<std::mutex> lock(m_tail_mutex);
        return m_tail;
    }

    node * pop_head() {
        std::lock_guard<std::mutex> lock(m_head_mutex);
        if (m_head == get_tail()) {
            return 0;
        }

        node * old_head = m_head;
        m_head = m_head->m_next;
        return old_head;
    }

    // Nodes are pushed to the tail and popped from the head.
    node * m_head;
    std::mutex m_head_mutex;
    node * m_tail;
    std::mutex m_tail_mutex;
};

#endif
