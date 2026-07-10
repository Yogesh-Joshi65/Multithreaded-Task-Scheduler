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
    Monitor(TaskQueue& q,
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

        using namespace std::chrono;

        auto startTime = steady_clock::now();

        int previousCompleted = 0;

        while (!stopFlag.load()) {

            std::this_thread::sleep_for(seconds(1));

#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif

            auto now = steady_clock::now();

            auto uptime =
                duration_cast<seconds>(now - startTime).count();

            int completed = completedTasks.load();
            int queueSize = queue.size();

            int throughput =
                completed - previousCompleted;

            previousCompleted = completed;

            double avgLatency = 0.0;

            if (completed > 0) {
                avgLatency =
                    static_cast<double>(totalLatency.load()) /
                    completed;
            }

            std::cout << "\n";
            std::cout << "=====================================================\n";
            std::cout << "          MULTITHREADED TASK SCHEDULER\n";
            std::cout << "=====================================================\n\n";

            std::cout << std::left
                      << std::setw(30)
                      << "Worker Threads"
                      << ": "
                      << workerCount
                      << '\n';

            std::cout << std::setw(30)
                      << "Queue Size"
                      << ": "
                      << queueSize
                      << '\n';

            std::cout << std::setw(30)
                      << "Completed Tasks"
                      << ": "
                      << completed
                      << '\n';

            std::cout << std::setw(30)
                      << "Throughput"
                      << ": "
                      << throughput
                      << " tasks/sec\n";

            std::cout << std::setw(30)
                      << "Average Latency"
                      << ": "
                      << std::fixed
                      << std::setprecision(2)
                      << avgLatency
                      << " ms\n";

            std::cout << std::setw(30)
                      << "System Uptime"
                      << ": "
                      << uptime
                      << " sec\n";

            std::cout << "\n";
            std::cout << "=====================================================\n";

            std::cout.flush();
        }
    }
};
