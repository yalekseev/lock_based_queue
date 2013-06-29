lock_based_queue
================

Thread-safe lock-based queue. The queue is optimized to allow more concurrency.
That is, push() and pop() can be run almost concurrently by different threads.

Usage
=====

```cpp

#include "queue.h"

int main() {
    lock_based::queue<int> queue;

    /* ... */

    return 0;
}
```
