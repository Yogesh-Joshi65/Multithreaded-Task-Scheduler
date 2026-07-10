#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

// ANSI Colors
#define RESET  "\033[0m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define CYAN   "\033[36m"
#define RED    "\033[31m"

class Monitor {
private:
    TaskQueue& queue;

    std::atomic<int>& completed;
    std::atomic<long long>& totalLatency;

    std::atomic<bool>& stopFlag;

    int workerCount;

public:
    Monitor(TaskQueue& q,
            std::atomic<int>& comp,
            std::atomic<long long>& latency,
            std::atomic<bool>& stop,
            int workers)
        : queue(q),
          completed(comp),
          totalLatency(latency),
          stopFlag(stop),
          workerCount(workers) {}

    void operator()() {

        int previousCompleted = 0;

        while (!stopFlag.load()) {

            std::this_thread::sleep_for(
                std::chrono::seconds(1));

#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif

            int currentCompleted = completed.load();

            int throughput =
                currentCompleted - previousCompleted;

            previousCompleted = currentCompleted;

            double avgLatency = 0.0;

            if (currentCompleted > 0) {
                avgLatency =
                    static_cast<double>(
                        totalLatency.load()) /
                    currentCompleted;
            }

            std::cout << CYAN
                      << "=============================================\n";
            std::cout
                << "      MULTITHREADED TASK SCHEDULER\n";
            std::cout
                << "=============================================\n"
                << RESET;

            std::cout << GREEN
                      << "Worker Threads : "
                      << RESET
                      << workerCount
                      << "\n";

            std::cout << GREEN
                      << "Queue Size     : "
                      << RESET
                      << queue.size()
                      << "\n";

            std::cout << GREEN
                      << "Completed      : "
                      << RESET
                      << currentCompleted
                      << "\n";

            std::cout << YELLOW
                      << "Throughput     : "
                      << RESET
                      << throughput
                      << " tasks/sec\n";

            std::cout << YELLOW
                      << "Avg Latency    : "
                      << RESET
                      << std::fixed
                      << std::setprecision(2)
                      << avgLatency
                      << " ms\n";

            std::cout << CYAN
                      << "=============================================\n"
                      << RESET;
        }
    }
};
