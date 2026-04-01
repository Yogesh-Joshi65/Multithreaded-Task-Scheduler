#pragma once
#include "TaskQueue.h"
#include <thread>
#include <future>
#include <iostream>
#include <atomic>

class Worker {
private:
    TaskQueue &queue;
    bool &stop_flag;
    std::atomic<int> &completed;
    std::atomic<long long> &total_latency;

public:
    Worker(TaskQueue &q, bool &stop,
           std::atomic<int> &comp,
           std::atomic<long long> &lat)
        : queue(q), stop_flag(stop),
          completed(comp), total_latency(lat) {}

    void operator()() {
        while (!stop_flag) {
            Task task = queue.pop();

            if (task.is_cancelled) continue;

            auto start = std::chrono::steady_clock::now();

            try {
                auto future = std::async(std::launch::async, task.func);

                if (future.wait_for(std::chrono::seconds(2)) == std::future_status::timeout) {
                    std::cout << "Task " << task.id << " timed out\n";
                } else {
                    future.get();
                }
            } catch (...) {
                std::cout << "Task " << task.id << " failed\n";
            }

            auto end = std::chrono::steady_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
                               end - task.enqueue_time)
                               .count();

            total_latency += latency;
            completed++;
        }
    }
};
