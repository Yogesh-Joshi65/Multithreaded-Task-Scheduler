#pragma once
#include "Task.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include<algorithm>

class TaskQueue {
private:
    std::vector<Task> heap;
    std::mutex mtx;
    std::condition_variable cv;

    static bool compare(const Task &a, const Task &b) {
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

public:
    void push(Task task) {
        std::unique_lock<std::mutex> lock(mtx);
        heap.push_back(std::move(task));
        heapify_up(heap.size() - 1);
        cv.notify_one();
    }

    Task pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return !heap.empty(); });

        Task top = std::move(heap[0]);
        heap[0] = heap.back();
        heap.pop_back();
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
        auto now = std::chrono::steady_clock::now();

        for (auto &task : heap) {
            auto wait_time = std::chrono::duration_cast<std::chrono::seconds>(
                                 now - task.enqueue_time)
                                 .count();
            task.effective_priority = task.base_priority + wait_time;
        }

        for (int i = heap.size() / 2 - 1; i >= 0; --i)
            heapify_down(i);
    }

    int size() {
        std::lock_guard<std::mutex> lock(mtx);
        return heap.size();
    }
};
