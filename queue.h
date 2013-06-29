#ifndef QUEUE_H
#define QUEUE_H

#include <condition_variable>
#include <atomic>
#include <limits>
#include <memory>
#include <mutex>

namespace lock_based {

template <typename T>
class queue {
private:
    enum { MAX_CAPACITY = std::numeric_limits<int>::max() };

    struct node {
        node() : m_next(0) { }

        explicit node(const T & data) : m_data(new T(data)), m_next(0) { }
        node(const node & other) = delete;
        node & operator=(const node & other) = delete;

        std::shared_ptr<T> m_data;
        node * m_next;
    };

public:
    explicit queue(int capacity = MAX_CAPACITY);
    queue(const queue & other) = delete;
    queue & operator=(const queue & other) = delete;

    ~queue();

    bool empty() const;

    void pop(T & val);
    bool try_pop(T & val);

    void push(const T & val);

private:
    node * get_tail();

    int m_capacity;
    std::atomic<int> m_size;

    // Nodes are pushed to the tail and popped from the head.
    node * m_head;
    std::mutex m_head_mutex;

    node * m_tail;
    std::mutex m_tail_mutex;

    std::condition_variable m_non_full_cond;
    std::condition_variable m_non_empty_cond;
};

template <typename T>
queue<T>::queue(int capacity)
    : m_capacity(capacity)
    , m_size(0)
    , m_head(new node)
    , m_tail(m_head) { }

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
    node * old_head = 0;

    {
        std::unique_lock<std::mutex> lock(m_head_mutex);
        m_non_empty_cond.wait(lock, [&](){ return m_head != get_tail(); });

        val = *(m_head->m_data);

        old_head = m_head;
        m_head = m_head->m_next;
        --m_size;
    }

    m_non_full_cond.notify_one();

    delete old_head;
}

template <typename T>
bool queue<T>::try_pop(T & val) {
    node * old_head = 0;

    {
        std::lock_guard<std::mutex> lock(m_head_mutex);
        if (m_head == get_tail()) {
            return false;
        }

        val = *(m_head->m_data);

        old_head = m_head;
        m_head = m_head->m_next;

        --m_size;
    }

    m_non_full_cond.notify_one();

    delete old_head;

    return true;
}

template <typename T>
void queue<T>::push(const T & val) {
    std::shared_ptr<T> new_data(new T(val));
    node * new_node = new node;

    {
        std::unique_lock<std::mutex> lock(m_tail_mutex);
        m_non_full_cond.wait(lock, [&](){ return m_size < m_capacity; });
        m_tail->m_data = new_data;
        m_tail->m_next = new_node;
        m_tail = new_node;
    }

    m_non_empty_cond.notify_one();
}

template <typename T>
typename queue<T>::node * queue<T>::get_tail() {
    std::lock_guard<std::mutex> lock(m_tail_mutex);
    return m_tail;
}

} // namespace lock_based

#endif
