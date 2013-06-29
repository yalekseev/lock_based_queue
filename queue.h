#ifndef QUEUE_H
#define QUEUE_H

#include <condition_variable>
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
    queue();
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    ~queue();

    bool empty() const;

    void pop(T & val);
    bool try_pop(T & val);

    void push(const T & val);

private:
    node * get_tail();

    // Nodes are pushed to the tail and popped from the head.
    node * m_head;
    std::mutex m_head_mutex;

    node * m_tail;
    std::mutex m_tail_mutex;

    std::condition_variable m_non_empty_cond;
};

template <typename T>
queue<T>::queue() : m_head(new node), m_tail(m_head) { }

template <typename T>
queue<T>::~queue() {
    while (m_head) {
      node * old_head = m_head;
      m_head = old_head->m_next;
      delete old_head;
    }
}

template <typename T>
bool queue<T>::empty() const {
    std::lock_guard<std::mutex> lock(m_head_mutex);
    return m_head == get_tail();
}

template <typename T>
void queue<T>::pop(T & val) {
    std::unique_lock<std::mutex> lock(m_head_mutex);
    m_non_empty_cond.wait(lock, [&](){ return m_head != get_tail(); });

    val = *(m_head->m_data);

    node * old_head = m_head;
    m_head = m_head->m_next;

    delete old_head;
}

template <typename T>
bool queue<T>::try_pop(T & val) {
    std::lock_guard<std::mutex> lock(m_head_mutex);
    if (m_head == get_tail()) {
        return false;
    }

    val = *(m_head->m_data);

    node * old_head = m_head;
    m_head = m_head->m_next;

    delete old_head;
    return true;
}

template <typename T>
void queue<T>::push(const T & val) {
    std::shared_ptr<T> new_data(new T(val));
    node * new_node = new node;

    std::lock_guard<std::mutex> lock(m_tail_mutex);
    m_tail->m_data = new_data;
    m_tail->m_next = new_node;
    m_tail = new_node;
}

template <typename T>
typename queue<T>::node * queue<T>::get_tail() {
    std::lock_guard<std::mutex> lock(m_tail_mutex);
    return m_tail;
}

#endif
