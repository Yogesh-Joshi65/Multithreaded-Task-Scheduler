#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <utility>

#include "Task.h"
#include "TaskQueue.h"
#include "Worker.h"
#include "Monitor.h"

class ThreadPool {
private:
    TaskQueue queue;

    std::vector<std::thread> workers;
    std::thread aging_thread;
    std::thread monitor_thread;

    std::atomic<bool> stop_flag{false};

    std::atomic<int> completed{0};
    std::atomic<long long> total_latency{0};

public:
    explicit ThreadPool(int n) {
        for (int i = 0; i < n; i++) {
            workers.emplace_back(
                Worker(queue,
                       stop_flag,
                       completed,
                       total_latency));
        }

        aging_thread = std::thread([this]() {
            while (!stop_flag.load()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                queue.apply_aging();
            }
        });

        monitor_thread = std::thread(
            Monitor(queue,
                    completed,
                    total_latency,
                    stop_flag,
                    n));
    }

    void submit(Task task) {
        queue.push(std::move(task));
    }

    void cancelTask(int id) {
        queue.cancel(id);
    }

    void shutdown() {
        stop_flag.store(true);

        queue.shutdown();

        for (auto &t : workers)
            if (t.joinable())
                t.join();

        if (aging_thread.joinable())
            aging_thread.join();

        if (monitor_thread.joinable())
            monitor_thread.join();
    }

    ~ThreadPool() {
        if (!stop_flag.load())
            shutdown();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};
