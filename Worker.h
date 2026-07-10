#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

class Worker {
private:
    TaskQueue& queue;

    std::atomic<bool>& stopFlag;
    std::atomic<int>& completedTasks;
    std::atomic<long long>& totalLatency;

public:
    Worker(TaskQueue& q,
           std::atomic<bool>& stop,
           std::atomic<int>& completed,
           std::atomic<long long>& latency)
        : queue(q),
          stopFlag(stop),
          completedTasks(completed),
          totalLatency(latency) {}

    void operator()() {

        while (!stopFlag.load()) {

            try {

                Task task = queue.pop();

                if (task.cancelled.load()) {
                    continue;
                }

                auto start =
                    std::chrono::steady_clock::now();

                try {
                    task.func();
                }
                catch (const std::exception& e) {
                    std::cout
                        << "[Worker] Task "
                        << task.id
                        << " failed: "
                        << e.what()
                        << "\n";
                }
                catch (...) {
                    std::cout
                        << "[Worker] Task "
                        << task.id
                        << " failed.\n";
                }

                auto finish =
                    std::chrono::steady_clock::now();

                auto latency =
                    std::chrono::duration_cast<
                        std::chrono::milliseconds>(
                        finish - task.enqueue_time)
                        .count();

                completedTasks.fetch_add(1);
                totalLatency.fetch_add(latency);

            }
            catch (...) {
                break;
            }
        }
    }
};
