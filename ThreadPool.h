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

    std::thread agingThread;
    std::thread monitorThread;

    std::atomic<bool> stopFlag{false};

    std::atomic<int> completedTasks{0};
    std::atomic<long long> totalLatency{0};

    int workerCount;

public:

    explicit ThreadPool(int threads)
        : workerCount(threads)
    {

        // Create Worker Threads
        for (int i = 0; i < workerCount; i++) {

            workers.emplace_back(

                Worker(
                    i + 1,
                    queue,
                    stopFlag,
                    completedTasks,
                    totalLatency)

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

        // Monitoring Thread
        monitorThread = std::thread(

            Monitor(
                queue,
                completedTasks,
                totalLatency,
                stopFlag,
                workerCount)

        );
    }

    //-------------------------------------------------

    void submit(Task task) {

        queue.push(std::move(task));

    }

    //-------------------------------------------------

    void cancelTask(int taskId) {

        queue.cancel(taskId);

    }

    //-------------------------------------------------

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

        std::cout << "\nScheduler Shutdown Successfully.\n";
    }

    //-------------------------------------------------

    ~ThreadPool() {

        shutdown();

    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};
