#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

class Worker {
private:
    int workerId;

    TaskQueue& queue;

    std::atomic<bool>& stopFlag;

    std::atomic<int>& completedTasks;
    std::atomic<long long>& totalLatency;

public:
    Worker(
        int id,
        TaskQueue& q,
        std::atomic<bool>& stop,
        std::atomic<int>& completed,
        std::atomic<long long>& latency)
        : workerId(id),
          queue(q),
          stopFlag(stop),
          completedTasks(completed),
          totalLatency(latency) {}

    void operator()() {

        while (!stopFlag.load()) {

            try {

                Task task = queue.pop();

                if (task.isCancelled()) {

                    std::cout
                        << "[Worker "
                        << workerId
                        << "] Task "
                        << task.id
                        << " cancelled.\n";

                    continue;
                }

                task.markRunning();

                auto start =
                    std::chrono::steady_clock::now();

                std::cout
                    << "[Worker "
                    << workerId
                    << "] Started Task "
                    << task.id
                    << " (Priority "
                    << task.effectivePriority
                    << ")\n";

                try {

                    task.work();

                    task.markCompleted();

                }
                catch (const std::exception& e) {

                    task.markFailed();

                    std::cout
                        << "[Worker "
                        << workerId
                        << "] Task "
                        << task.id
                        << " failed : "
                        << e.what()
                        << '\n';

                    continue;
                }
                catch (...) {

                    task.markFailed();

                    std::cout
                        << "[Worker "
                        << workerId
                        << "] Task "
                        << task.id
                        << " failed.\n";

                    continue;
                }

                auto finish =
                    std::chrono::steady_clock::now();

                auto latency =
                    std::chrono::duration_cast<
                        std::chrono::milliseconds>(
                        finish -
                        task.enqueueTime)
                        .count();

                completedTasks.fetch_add(1);

                totalLatency.fetch_add(latency);

                std::cout
                    << "[Worker "
                    << workerId
                    << "] Finished Task "
                    << task.id
                    << " ("
                    << latency
                    << " ms)\n";

            }
            catch (...) {

                break;
            }
        }

        std::cout
            << "[Worker "
            << workerId
            << "] Stopped.\n";
    }
};
