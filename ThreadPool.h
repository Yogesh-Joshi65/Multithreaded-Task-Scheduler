#pragma once
#include "TaskQueue.h"
#include "Worker.h"
#include "Monitor.h"
#include <vector>
#include <atomic>

class ThreadPool {
private:
    TaskQueue queue;
    std::vector<std::thread> workers;
    std::thread aging_thread;
    std::thread monitor_thread;

    bool stop_flag = false;

    std::atomic<int> completed{0};
    std::atomic<long long> total_latency{0};

public:
    ThreadPool(int n) {
        for (int i = 0; i < n; i++) {
            workers.emplace_back(
                Worker(queue, stop_flag, completed, total_latency));
        }

        aging_thread = std::thread([&]() {
            while (!stop_flag) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                queue.apply_aging();
            }
        });

        monitor_thread = std::thread(
            Monitor(queue, completed, total_latency, stop_flag, n));
    }

    void submit(Task task) {
        queue.push(task);
    }

    void cancelTask(int id) {
        queue.cancel(id);
    }

    void shutdown() {
        stop_flag = true;
        for (auto &t : workers) t.join();
        aging_thread.join();
        monitor_thread.join();
    }
};
