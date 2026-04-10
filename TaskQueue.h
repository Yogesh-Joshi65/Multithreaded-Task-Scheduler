#pragma once
#include "Task.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <chrono>
#include <stdexcept>

class TaskQueue {
private:
std::vector<Task> heap;
std::mutex mtx;
std::condition_variable cv;
bool stop_flag = false;

```
static bool compare(const Task &a, const Task &b) {
    // Higher priority should come first → max heap behavior
    if (a.effective_priority == b.effective_priority) {
        if (a.base_priority == b.base_priority) {
            return a.enqueue_time > b.enqueue_time; // older first
        }
        return a.base_priority < b.base_priority; // urgency wins
    }
    return a.effective_priority < b.effective_priority;
}

void heapify_up(int i) {
    while (i > 0 && compare(heap[(i - 1) / 2], heap[i])) {
        std::swap(heap[i], heap[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void heapify_down(int i) {
    int n = heap.size();
    while (true) {
        int largest = i;
        int l = 2 * i + 1, r = 2 * i + 2;

        if (l < n && compare(heap[largest], heap[l])) largest = l;
        if (r < n && compare(heap[largest], heap[r])) largest = r;

        if (largest == i) break;

        std::swap(heap[i], heap[largest]);
        i = largest;
    }
}
```

public:
void push(Task task) {
std::unique_lock[std::mutex](std::mutex) lock(mtx);
heap.push_back(std::move(task));
heapify_up(heap.size() - 1);
cv.notify_one();
}

```
Task pop() {
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock, [&]() {
        return stop_flag || !heap.empty();
    });

    if (stop_flag && heap.empty()) {
        throw std::runtime_error("Queue shutdown");
    }

    Task top = std::move(heap[0]);

    heap[0] = heap.back();
    heap.pop_back();

    if (!heap.empty())
        heapify_down(0);

    return top;
}

void cancel(int task_id) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto &t : heap) {
        if (t.id == task_id) {
            t.is_cancelled = true;
        }
    }
}

void apply_aging() {
    std::unique_lock<std::mutex> lock(mtx);

    const int MAX_PRIORITY_CAP = 100;
    auto now = std::chrono::steady_clock::now();

    for (auto &task : heap) {
        auto wait_time = std::chrono::duration_cast<std::chrono::seconds>(
                             now - task.enqueue_time)
                             .count();

        task.effective_priority = std::min(
            task.base_priority + (int)wait_time,
            MAX_PRIORITY_CAP
        );
    }

    for (int i = (int)heap.size() / 2 - 1; i >= 0; --i)
        heapify_down(i);
}

int size() {
    std::lock_guard<std::mutex> lock(mtx);
    return heap.size();
}

void shutdown() {
    std::lock_guard<std::mutex> lock(mtx);
    stop_flag = true;
    cv.notify_all(); // wake all waiting threads
}
```

};
