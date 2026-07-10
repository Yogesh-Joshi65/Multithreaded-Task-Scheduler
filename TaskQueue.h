#pragma once

#include "Task.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <vector>

class TaskQueue {
private:

    // ----------------------------------------------------
    // Binary Max Heap
    //
    // Stores all waiting tasks.
    // Highest effective priority always remains at index 0.
    // ----------------------------------------------------
    std::vector<Task> heap;

    // Protects heap from concurrent access.
    std::mutex mtx;

    // Used by worker threads to sleep when queue is empty
    // and wake up when new tasks arrive.
    std::condition_variable cv;

    // Indicates scheduler shutdown.
    // Once true, workers stop waiting for new tasks.
    bool stopped = false;

    // ----------------------------------------------------
    // Comparator for Max Heap
    //
    // Returns true if first task has LOWER priority than
    // second task.
    //
    // If priorities are equal,
    // older task (smaller enqueueTime) gets preference.
    // ----------------------------------------------------
    static bool compare(const Task& a, const Task& b) {

        // Same priority -> FIFO ordering
        if (a.effectivePriority == b.effectivePriority) {
            return a.enqueueTime > b.enqueueTime;
        }

        // Higher priority wins
        return a.effectivePriority < b.effectivePriority;
    }

    // ----------------------------------------------------
    // Heapify Up
    //
    // Called after inserting a new task.
    // Moves task upward until heap property is restored.
    //
    // Time Complexity : O(log n)
    // ----------------------------------------------------
    void heapifyUp(int index) {

        while (index > 0) {

            int parent = (index - 1) / 2;

            // Parent already has higher priority
            if (!compare(heap[parent], heap[index]))
                break;

            std::swap(heap[parent], heap[index]);

            index = parent;
        }
    }

    // ----------------------------------------------------
    // Heapify Down
    //
    // Called after removing root task.
    // Moves replacement node downward until heap property
    // becomes valid again.
    //
    // Time Complexity : O(log n)
    // ----------------------------------------------------
    void heapifyDown(int index) {

        int n = heap.size();

        while (true) {

            int largest = index;

            int left = 2 * index + 1;
            int right = 2 * index + 2;

            // Compare left child
            if (left < n && compare(heap[largest], heap[left]))
                largest = left;

            // Compare right child
            if (right < n && compare(heap[largest], heap[right]))
                largest = right;

            // Heap property satisfied
            if (largest == index)
                break;

            std::swap(heap[index], heap[largest]);

            index = largest;
        }
    }

public:

    TaskQueue() = default;

    // ----------------------------------------------------
    // Insert new task into queue.
    //
    // Lock is required because multiple producer threads
    // may insert simultaneously.
    //
    // After insertion,
    // notify one sleeping worker thread.
    //
    // Complexity : O(log n)
    // ----------------------------------------------------
    void push(Task task) {

        {
            std::lock_guard<std::mutex> lock(mtx);

            heap.push_back(std::move(task));

            heapifyUp(heap.size() - 1);
        }

        // Wake one worker waiting on condition variable
        cv.notify_one();
    }

    // ----------------------------------------------------
    // Remove highest-priority task.
    //
    // If queue is empty,
    // worker thread sleeps efficiently using
    // condition_variable instead of busy waiting.
    //
    // Throws exception when scheduler is shutting down.
    //
    // Complexity : O(log n)
    // ----------------------------------------------------
    Task pop() {

        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [&]() {

            return stopped || !heap.empty();

        });

        // Scheduler is shutting down
        if (stopped && heap.empty())
            throw std::runtime_error("Queue stopped");

        // Highest priority task
        Task task = std::move(heap.front());

        // Last remaining element
        if (heap.size() == 1) {

            heap.pop_back();

            return task;
        }

        // Replace root with last node
        heap.front() = std::move(heap.back());

        heap.pop_back();

        // Restore heap property
        heapifyDown(0);

        return task;
    }

    // ----------------------------------------------------
    // Cancel a task before execution.
    //
    // Worker thread checks cancellation flag before
    // executing the task.
    //
    // Complexity : O(n)
    // ----------------------------------------------------
    void cancel(int taskId) {

        std::lock_guard<std::mutex> lock(mtx);

        for (auto &task : heap) {

            if (task.id == taskId) {

                task.cancel();

                break;
            }
        }
    }

    // ----------------------------------------------------
    // Priority Aging
    //
    // Prevents starvation.
    //
    // Every second spent waiting increases effective
    // priority by one.
    //
    // Priority is capped at MAX_PRIORITY.
    //
    // Heap is rebuilt after updating priorities.
    // ----------------------------------------------------
    void applyAging() {

        std::lock_guard<std::mutex> lock(mtx);

        auto now = std::chrono::steady_clock::now();

        constexpr int MAX_PRIORITY = 100;

        for (auto &task : heap) {

            auto waited =
                std::chrono::duration_cast<
                    std::chrono::seconds>(
                        now - task.enqueueTime).count();

            task.effectivePriority =
                std::min(
                    task.basePriority +
                    static_cast<int>(waited),
                    MAX_PRIORITY);
        }

        // Rebuild heap because priorities changed
        for (int i = heap.size() / 2 - 1; i >= 0; --i)
            heapifyDown(i);
    }

    // ----------------------------------------------------
    // Returns current number of waiting tasks.
    // Thread-safe.
    // ----------------------------------------------------
    int size() {

        std::lock_guard<std::mutex> lock(mtx);

        return heap.size();
    }

    // ----------------------------------------------------
    // Returns true if queue has no tasks.
    // Thread-safe.
    // ----------------------------------------------------
    bool empty() {

        std::lock_guard<std::mutex> lock(mtx);

        return heap.empty();
    }

    // ----------------------------------------------------
    // Stops scheduler.
    //
    // All waiting worker threads wake up.
    // pop() throws exception so workers terminate
    // gracefully.
    // ----------------------------------------------------
    void shutdown() {

        {
            std::lock_guard<std::mutex> lock(mtx);

            stopped = true;
        }

        // Wake every sleeping worker thread
        cv.notify_all();
    }
};
