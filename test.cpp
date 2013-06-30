#include "queue.h"

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <atomic>
#include <thread>
#include <mutex>
#include <set>

std::mutex produced_mutex;
std::set<int> produced;

std::mutex consumed_mutex;
std::set<int> consumed;

void produce(lock_based::queue<int> & q) {
    for (std::size_t i = 0; i < 1000; ++i) {
        int val = std::rand();
        q.push(val);

        {
            std::lock_guard<std::mutex> lock(produced_mutex);
            produced.insert(val);
        }

        std::this_thread::yield();
    }
}

void consume(lock_based::queue<int> & q) {
    while (true) {
        int val;
        q.pop(val);

        if (val == -1) {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(consumed_mutex);
            consumed.insert(val);
        }

        std::this_thread::yield();
    }
}

int main() {
    std::srand(123);

    for (std::size_t i = 0; i < 100; ++i) {
        lock_based::queue<int> queue(10000);

        std::thread producer1(std::bind(produce, std::ref(queue)));
        std::thread producer2(std::bind(produce, std::ref(queue)));
        std::thread producer3(std::bind(produce, std::ref(queue)));
        std::thread consumer1(std::bind(consume, std::ref(queue)));
        std::thread consumer2(std::bind(consume, std::ref(queue)));

        producer1.join();
        producer2.join();
        producer3.join();

        while (!queue.empty()) {
            std::this_thread::yield();
        }

        queue.push(-1);
        queue.push(-1);

        consumer1.join();
        consumer2.join();

        assert(produced == consumed);
    }

    return 0;
}
