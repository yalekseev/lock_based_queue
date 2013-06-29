lock_based_queue
================

Thread-safe lock-based bounded queue. The queue is optimized to allow better
concurrency. In particular push() and pop() can be run almost concurrently
by different threads.

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
