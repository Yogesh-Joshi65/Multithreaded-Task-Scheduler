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
    std::vector<Task> heap;

    std::mutex mtx;
    std::condition_variable cv;

    bool stopped = false;

    // Higher priority comes first
    static bool compare(const Task& a, const Task& b) {

        if (a.effectivePriority == b.effectivePriority) {
            return a.enqueueTime > b.enqueueTime;
        }

        return a.effectivePriority < b.effectivePriority;
    }

    void heapifyUp(int index) {

        while (index > 0) {

            int parent = (index - 1) / 2;

            if (!compare(heap[parent], heap[index]))
                break;

            std::swap(heap[parent], heap[index]);

            index = parent;
        }
    }

    void heapifyDown(int index) {

        int n = heap.size();

        while (true) {

            int largest = index;

            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < n && compare(heap[largest], heap[left]))
                largest = left;

            if (right < n && compare(heap[largest], heap[right]))
                largest = right;

            if (largest == index)
                break;

            std::swap(heap[index], heap[largest]);

            index = largest;
        }
    }

public:

    TaskQueue() = default;

    void push(Task task) {

        {
            std::lock_guard<std::mutex> lock(mtx);

            heap.push_back(std::move(task));

            heapifyUp(heap.size() - 1);
        }

        cv.notify_one();
    }

    Task pop() {

        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [&]() {

            return stopped || !heap.empty();

        });

        if (stopped && heap.empty())
            throw std::runtime_error("Queue stopped");

        Task task = std::move(heap.front());

        if (heap.size() == 1) {

            heap.pop_back();

            return task;
        }

        heap.front() = std::move(heap.back());

        heap.pop_back();

        heapifyDown(0);

        return task;
    }

    void cancel(int taskId) {

        std::lock_guard<std::mutex> lock(mtx);

        for (auto &task : heap) {

            if (task.id == taskId) {

                task.cancel();

                break;
            }
        }
    }

    void applyAging() {

        std::lock_guard<std::mutex> lock(mtx);

        auto now = std::chrono::steady_clock::now();

        constexpr int MAX_PRIORITY = 100;

        for (auto &task : heap) {

            auto waited = std::chrono::duration_cast<
                std::chrono::seconds>(
                    now - task.enqueueTime).count();

            task.effectivePriority =
                std::min(
                    task.basePriority +
                    static_cast<int>(waited),
                    MAX_PRIORITY);
        }

        for (int i = heap.size() / 2 - 1; i >= 0; --i)
            heapifyDown(i);
    }

    int size() {

        std::lock_guard<std::mutex> lock(mtx);

        return heap.size();
    }

    bool empty() {

        std::lock_guard<std::mutex> lock(mtx);

        return heap.empty();
    }

    void shutdown() {

        {
            std::lock_guard<std::mutex> lock(mtx);

            stopped = true;
        }

        cv.notify_all();
    }
};
