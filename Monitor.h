#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

class Monitor {
private:
    TaskQueue& queue;

    std::atomic<int>& completedTasks;
    std::atomic<long long>& totalLatency;

    std::atomic<bool>& stopFlag;

    int workerCount;

public:
    Monitor(
        TaskQueue& q,
        std::atomic<int>& completed,
        std::atomic<long long>& latency,
        std::atomic<bool>& stop,
        int workers)
        : queue(q),
          completedTasks(completed),
          totalLatency(latency),
          stopFlag(stop),
          workerCount(workers) {}

    void operator()() {

        int previousCompleted = 0;

        auto start =
            std::chrono::steady_clock::now();

        while (!stopFlag.load()) {

            std::this_thread::sleep_for(
                std::chrono::seconds(1));

#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif

            auto now =
                std::chrono::steady_clock::now();

            auto uptime =
                std::chrono::duration_cast<
                    std::chrono::seconds>(
                    now - start)
                    .count();

            int completed =
                completedTasks.load();

            int queueSize =
                queue.size();

            int throughput =
                completed - previousCompleted;

            previousCompleted = completed;

            double avgLatency = 0.0;

            if (completed > 0) {

                avgLatency =
                    static_cast<double>(
                        totalLatency.load())
                    / completed;
            }

            std::cout << "\n";
            std::cout << "=====================================================\n";
            std::cout << "        MULTITHREADED TASK SCHEDULER\n";
            std::cout << "=====================================================\n\n";

            std::cout << std::left
                      << std::setw(25)
                      << "Worker Threads"
                      << ": "
                      << workerCount
                      << '\n';

            std::cout << std::setw(25)
                      << "Queue Size"
                      << ": "
                      << queueSize
                      << '\n';

            std::cout << std::setw(25)
                      << "Completed Tasks"
                      << ": "
                      << completed
                      << '\n';

            std::cout << std::setw(25)
                      << "Throughput"
                      << ": "
                      << throughput
                      << " tasks/sec\n";

            std::cout << std::setw(25)
                      << "Average Latency"
                      << ": "
                      << std::fixed
                      << std::setprecision(2)
                      << avgLatency
                      << " ms\n";

            std::cout << std::setw(25)
                      << "System Uptime"
                      << ": "
                      << uptime
                      << " sec\n";

            std::cout << "\n";
            std::cout << "=====================================================\n";
        }
    }
};
