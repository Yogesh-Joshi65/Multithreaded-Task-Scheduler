#pragma once

#include "Task.h"

#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include <stdexcept>

class TaskQueue {
private:
    std::vector<Task> heap;

    std::mutex mtx;
    std::condition_variable cv;

    bool stopped = false;

    static bool compare(const Task& a, const Task& b) {
        if (a.effective_priority == b.effective_priority) {
            return a.enqueue_time > b.enqueue_time; // older task wins
        }
        return a.effective_priority < b.effective_priority;
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

        Task top = std::move(heap.front());

        if (heap.size() == 1) {
            heap.pop_back();
            return top;
        }

        heap.front() = std::move(heap.back());
        heap.pop_back();

        heapifyDown(0);

        return top;
    }

    void cancel(int taskId) {
        std::lock_guard<std::mutex> lock(mtx);

        for (auto& task : heap) {
            if (task.id == taskId) {
                task.cancelled.store(true);
                return;
            }
        }
    }

    void applyAging() {
        std::lock_guard<std::mutex> lock(mtx);

        auto now = std::chrono::steady_clock::now();

        for (auto& task : heap) {

            auto wait =
                std::chrono::duration_cast<std::chrono::seconds>(
                    now - task.enqueue_time).count();

            task.effective_priority =
                task.base_priority + static_cast<int>(wait);
        }

        for (int i = heap.size() / 2 - 1; i >= 0; --i)
            heapifyDown(i);
    }

    int size() {
        std::lock_guard<std::mutex> lock(mtx);
        return static_cast<int>(heap.size());
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stopped = true;
        }

        cv.notify_all();
    }
};
