#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

class Worker {
private:

    // ----------------------------------------------------
    // Unique identifier for this worker thread.
    // Useful for debugging and monitoring.
    // ----------------------------------------------------
    int workerId;

    // Shared task queue from which all workers
    // fetch tasks.
    TaskQueue& queue;

    // Global shutdown flag.
    // When true, worker exits its execution loop.
    std::atomic<bool>& stopFlag;

    // Shared statistics.
    // Updated after successful task completion.
    std::atomic<int>& completedTasks;
    std::atomic<long long>& totalLatency;

public:

    // ----------------------------------------------------
    // Constructor
    //
    // Initializes references to shared resources.
    // ----------------------------------------------------
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

    // ----------------------------------------------------
    // Function Call Operator
    //
    // This makes Worker a callable object (functor),
    // allowing it to be passed directly to std::thread.
    //
    // Example:
    // std::thread t(Worker(...));
    // ----------------------------------------------------
    void operator()() {

        // Keep processing tasks until scheduler stops.
        while (!stopFlag.load()) {

            try {

                // Remove the highest-priority task.
                // If queue is empty, worker sleeps until
                // a task is available.
                Task task = queue.pop();

                // Skip execution if task was cancelled.
                if (task.isCancelled()) {

                    std::cout
                        << "[Worker "
                        << workerId
                        << "] Task "
                        << task.id
                        << " cancelled.\n";

                    continue;
                }

                // Update task state.
                task.markRunning();

                // Record execution start time.
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

                    // Execute user-provided task.
                    task.work();

                    // Mark successful completion.
                    task.markCompleted();

                }
                catch (const std::exception& e) {

                    // Handle standard exceptions.
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

                    // Handle unknown exceptions.
                    task.markFailed();

                    std::cout
                        << "[Worker "
                        << workerId
                        << "] Task "
                        << task.id
                        << " failed.\n";

                    continue;
                }

                // Record task completion time.
                auto finish =
                    std::chrono::steady_clock::now();

                // ------------------------------------------------
                // Calculate total latency.
                //
                // Latency = Finish Time - Enqueue Time
                //
                // Includes:
                // - Waiting time in queue
                // - Actual execution time
                // ------------------------------------------------
                auto latency =
                    std::chrono::duration_cast<
                        std::chrono::milliseconds>(
                        finish -
                        task.enqueueTime)
                        .count();

                // Update shared scheduler statistics.
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

                // queue.pop() throws when scheduler
                // is shutting down.
                break;
            }
        }

        std::cout
            << "[Worker "
            << workerId
            << "] Stopped.\n";
    }
};
