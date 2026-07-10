#pragma once

#include "TaskQueue.h"
#include "Worker.h"
#include "Monitor.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

class ThreadPool {
private:

    // ----------------------------------------------------
    // Shared priority queue.
    // All worker threads fetch tasks from this queue.
    // ----------------------------------------------------
    TaskQueue queue;

    // Stores all worker threads.
    std::vector<std::thread> workers;

    // Background thread responsible for
    // periodically applying priority aging.
    std::thread agingThread;

    // Background thread responsible for
    // continuously displaying scheduler statistics.
    std::thread monitorThread;

    // Global stop flag.
    // All threads periodically check this flag
    // to terminate gracefully.
    std::atomic<bool> stopFlag{false};

    // Total number of completed tasks.
    // Atomic because multiple workers update it.
    std::atomic<int> completedTasks{0};

    // Sum of execution latency of all completed tasks.
    // Used to calculate average latency.
    std::atomic<long long> totalLatency{0};

    // Number of worker threads.
    int workerCount;

public:

    // ----------------------------------------------------
    // Constructor
    //
    // Creates:
    // 1. Worker Threads
    // 2. Aging Thread
    // 3. Monitoring Thread
    // ----------------------------------------------------
    explicit ThreadPool(int threads)
        : workerCount(threads)
    {

        // ------------------------------------------------
        // Create Worker Threads
        //
        // Each worker continuously fetches tasks from the
        // shared queue until scheduler shuts down.
        // ------------------------------------------------
        for (int i = 0; i < workerCount; i++) {

            workers.emplace_back(

                Worker(
                    i + 1,             // Worker ID
                    queue,             // Shared task queue
                    stopFlag,          // Shutdown signal
                    completedTasks,    // Shared statistics
                    totalLatency)      // Shared statistics

            );
        }

        // ------------------------------------------------
        // Aging Thread
        //
        // Runs every second.
        // Increases priority of waiting tasks to prevent
        // starvation.
        // ------------------------------------------------
        agingThread = std::thread([this]() {

            while (!stopFlag.load()) {

                std::this_thread::sleep_for(
                    std::chrono::seconds(1));

                queue.applyAging();
            }
        });

        // ------------------------------------------------
        // Monitoring Thread
        //
        // Continuously prints scheduler metrics like:
        // - Queue size
        // - Completed tasks
        // - Average latency
        // ------------------------------------------------
        monitorThread = std::thread(

            Monitor(
                queue,
                completedTasks,
                totalLatency,
                stopFlag,
                workerCount)

        );
    }

    // ----------------------------------------------------
    // Submit a new task.
    //
    // Task is inserted into priority queue.
    // Waiting worker thread is notified.
    // ----------------------------------------------------
    void submit(Task task) {

        queue.push(std::move(task));

    }

    // ----------------------------------------------------
    // Cancel a task before execution.
    //
    // Marks task as cancelled.
    // Worker thread skips cancelled tasks.
    // ----------------------------------------------------
    void cancelTask(int taskId) {

        queue.cancel(taskId);

    }

    // ----------------------------------------------------
    // Gracefully shuts down scheduler.
    //
    // Steps:
    // 1. Set stop flag.
    // 2. Stop task queue.
    // 3. Wake sleeping workers.
    // 4. Wait for every thread to finish.
    // ----------------------------------------------------
    void shutdown() {

        bool expected = false;

        // Prevent multiple shutdown calls.
        if (!stopFlag.compare_exchange_strong(expected, true))
            return;

        // Stop queue and wake waiting workers.
        queue.shutdown();

        // Wait for every worker thread.
        for (auto &worker : workers) {

            if (worker.joinable())
                worker.join();
        }

        // Wait for aging thread.
        if (agingThread.joinable())
            agingThread.join();

        // Wait for monitoring thread.
        if (monitorThread.joinable())
            monitorThread.join();

        std::cout << "\nScheduler Shutdown Successfully.\n";
    }

    // ----------------------------------------------------
    // Destructor
    //
    // Ensures all threads terminate even if user forgets
    // to explicitly call shutdown().
    // ----------------------------------------------------
    ~ThreadPool() {

        shutdown();

    }

    // ThreadPool owns threads and shared resources.
    // Copying is disabled.
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};
