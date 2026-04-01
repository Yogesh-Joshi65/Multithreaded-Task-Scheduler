#pragma once
#include "TaskQueue.h"
#include <atomic>
#include <thread>
#include <iostream>
#include <iomanip>

#define CYAN "\033[36m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

class Monitor {
private:
    TaskQueue &queue;
    std::atomic<int> &completed;
    std::atomic<long long> &total_latency;
    bool &stop_flag;
    int thread_count;

public:
    Monitor(TaskQueue &q,
            std::atomic<int> &comp,
            std::atomic<long long> &lat,
            bool &stop,
            int threads)
        : queue(q), completed(comp), total_latency(lat),
          stop_flag(stop), thread_count(threads) {}

    void operator()() {
        using namespace std::chrono;

        auto start_time = steady_clock::now();

        while (!stop_flag) {
            std::this_thread::sleep_for(seconds(1));

            auto now = steady_clock::now();
            double elapsed = duration_cast<seconds>(now - start_time).count();

            int comp = completed.load();
            int qsize = queue.size();

            double throughput = (elapsed > 0) ? comp / elapsed : 0;
            double avg_latency = (comp > 0) ? total_latency.load() / comp : 0;

            std::cout << "\033[2J\033[H";

            std::cout << CYAN << "=========== TASK SCHEDULER DASHBOARD ===========\n" << RESET;
            std::cout << GREEN << "Queue Size        : " << RESET << qsize << "\n";
            std::cout << GREEN << "Completed Tasks   : " << RESET << comp << "\n";
            std::cout << YELLOW << "Throughput        : " << RESET
                      << std::fixed << std::setprecision(2)
                      << throughput << " tasks/sec\n";
            std::cout << YELLOW << "Avg Latency       : " << RESET
                      << avg_latency << " ms\n";
            std::cout << CYAN << "Active Threads    : " << RESET << thread_count << "\n";
            std::cout << CYAN << "================================================\n" << RESET;
        }
    }
};
