#include "queue.h"

#include <cstdlib>
#include <thread>
#include <atomic>

std::atomic<bool> done(false);

void produce(queue<int> & q) {
    for (std::size_t i = 0; i < 1000; ++i) {
        q.push(std::rand());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    done = true;
}

void consume(queue<int> & q) {
    while (!done) {
        int val;
        q.try_pop(val);
        std::this_thread::yield();
    }
}

int main() {
    std::srand(123);

    queue<int> q;
    std::thread producer(std::bind(produce, std::ref(q)));
    std::thread consumer1(std::bind(consume, std::ref(q)));
    std::thread consumer2(std::bind(consume, std::ref(q)));

    producer.join();
    consumer1.join();
    consumer2.join();

    return 0;
}
