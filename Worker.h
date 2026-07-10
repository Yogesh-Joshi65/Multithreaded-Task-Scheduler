#pragma once

#include "TaskQueue.h"

#include <thread>
#include <future>
#include <iostream>
#include <chrono>
#include <atomic>

class Worker {
private:
    TaskQueue& queue;
    std::atomic<bool>& stop_flag;

    std::atomic<int>& completed;
    std::atomic<long long>& total_latency;

public:
    Worker(TaskQueue& q,
           std::atomic<bool>& stop,
           std::atomic<int>& comp,
           std::atomic<long long>& latency)
        : queue(q),
          stop_flag(stop),
          completed(comp),
          total_latency(latency) {}

    void operator()() {
        while (!stop_flag.load()) {

            Task task(0, "", 0, [] {});

            try {
                task = queue.pop();
            }
            catch (const std::runtime_error&) {
                break;          // Queue shutdown
            }

            if (task.is_cancelled.load())
                continue;

            try {
                auto future =
                    std::async(std::launch::async, task.func);

                if (future.wait_for(std::chrono::seconds(2))
                    == std::future_status::timeout) {

                    std::cout << "Task "
                              << task.id
                              << " timed out\n";
                }
                else {
                    future.get();
                }
            }
            catch (...) {
                std::cout << "Task "
                          << task.id
                          << " failed\n";
            }

            auto end = std::chrono::steady_clock::now();

            auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    end - task.enqueue_time)
                    .count();

            total_latency.fetch_add(latency);
            completed.fetch_add(1);
        }
    }
};
