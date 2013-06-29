#ifndef QUEUE_H
#define QUEUE_H

#include <memory>

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
        if (m_head == m_tail) {
            return false;
        }

        val = *(m_head->m_data);

        node * old_head = m_head;
        m_head = old_head->m_next;
        delete old_head;

        return true;
    }

    void push(const T & val) {
        node * new_node = new node(val);

        m_tail->m_next = new_node;
        m_tail = new_node;
    }

private:
    // Nodes are pushed to the tail and popped from the head.
    node * m_head;
    node * m_tail;
};

#endif
