#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

class Monitor {
private:

    // Shared task queue
    TaskQueue& queue;

    // Shared statistics
    std::atomic<int>& completedTasks;
    std::atomic<long long>& totalLatency;

    // Global shutdown flag
    std::atomic<bool>& stopFlag;

    // Total worker threads
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

    //------------------------------------------------------

    void operator()() {

        using namespace std::chrono;

        auto startTime = steady_clock::now();

        int previousCompleted = 0;

        while (!stopFlag.load()) {

            // Update every 2 seconds
            std::this_thread::sleep_for(seconds(2));

            auto now = steady_clock::now();

            long long uptime =
                duration_cast<seconds>(
                    now - startTime).count();

            int completed = completedTasks.load();

            int queueSize = queue.size();

            int throughput =
                completed - previousCompleted;

            previousCompleted = completed;

            double avgLatency = 0.0;

            if (completed > 0) {

                avgLatency =
                    static_cast<double>(
                        totalLatency.load()) / completed;
            }

            double utilization = 0.0;

            if (workerCount > 0) {

                utilization =
                    std::min(
                        100.0,
                        (throughput * 100.0) /
                        workerCount);
            }

            //--------------------------------------------------

            std::cout << "\n\n";
            std::cout << "=============================================================\n";
            std::cout << "            MULTITHREADED TASK SCHEDULER MONITOR\n";
            std::cout << "=============================================================\n";

            std::cout << std::left;

            std::cout << std::setw(30)
                      << "System Uptime"
                      << ": "
                      << uptime
                      << " sec\n";

            std::cout << std::setw(30)
                      << "Worker Threads"
                      << ": "
                      << workerCount
                      << '\n';

            std::cout << std::setw(30)
                      << "Scheduler State"
                      << ": "
                      << (queue.empty() ? "Idle" : "Processing")
                      << '\n';

            std::cout << std::setw(30)
                      << "Waiting Tasks"
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
                      << " tasks / 2 sec\n";

            std::cout << std::setw(30)
                      << "Average Latency"
                      << ": "
                      << std::fixed
                      << std::setprecision(2)
                      << avgLatency
                      << " ms\n";

            std::cout << std::setw(30)
                      << "Worker Utilization"
                      << ": "
                      << std::fixed
                      << std::setprecision(1)
                      << utilization
                      << "%\n";

            std::cout << "=============================================================\n";
            std::cout.flush();
        }

        std::cout
            << "\n[Monitor] Monitoring stopped.\n";
    }
};
