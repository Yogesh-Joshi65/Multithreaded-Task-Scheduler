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
    TaskQueue queue;

    std::vector<std::thread> workers;

    std::thread monitorThread;
    std::thread agingThread;

    std::atomic<bool> stopFlag{false};

    std::atomic<int> completedTasks{0};
    std::atomic<long long> totalLatency{0};

public:

    explicit ThreadPool(int threadCount) {

        // Create worker threads
        for (int i = 0; i < threadCount; i++) {

            workers.emplace_back(
                Worker(
                    queue,
                    stopFlag,
                    completedTasks,
                    totalLatency
                )
            );
        }

        // Aging Thread
        agingThread = std::thread([this]() {

            while (!stopFlag.load()) {

                std::this_thread::sleep_for(
                    std::chrono::seconds(1));

                queue.applyAging();
            }
        });

        // Monitor Thread
        monitorThread = std::thread(
            Monitor(
                queue,
                completedTasks,
                totalLatency,
                stopFlag,
                threadCount));
    }

    void submit(Task task) {
        queue.push(std::move(task));
    }

    void cancelTask(int taskId) {
        queue.cancel(taskId);
    }

    void shutdown() {

        bool expected = false;

        if (!stopFlag.compare_exchange_strong(expected, true))
            return;

        queue.shutdown();

        for (auto &worker : workers) {

            if (worker.joinable())
                worker.join();
        }

        if (agingThread.joinable())
            agingThread.join();

        if (monitorThread.joinable())
            monitorThread.join();
    }

    ~ThreadPool() {
        shutdown();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};
