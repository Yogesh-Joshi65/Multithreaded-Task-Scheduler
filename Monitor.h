#pragma once

#include "TaskQueue.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

// ANSI color codes
#define RESET  "\033[0m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define CYAN   "\033[36m"

class Monitor {
private:
    TaskQueue& queue;
    std::atomic<int>& completed;
    std::atomic<long long>& total_latency;
    std::atomic<bool>& stop_flag;
    int thread_count;

public:
    Monitor(TaskQueue& q,
            std::atomic<int>& comp,
            std::atomic<long long>& lat,
            std::atomic<bool>& stop,
            int threads)
        : queue(q),
          completed(comp),
          total_latency(lat),
          stop_flag(stop),
          thread_count(threads) {}

    void operator()() {
        using namespace std::chrono;

        auto start_time = steady_clock::now();
        int last_completed = 0;

        while (!stop_flag.load()) {
            std::this_thread::sleep_for(seconds(1));

            auto now = steady_clock::now();
            [[maybe_unused]] double elapsed =
                duration_cast<seconds>(now - start_time).count();

            int comp = completed.load();
            int qsize = queue.size();

            int diff = comp - last_completed;
            double throughput = static_cast<double>(diff);
            last_completed = comp;

            double avg_latency =
                (comp > 0)
                    ? static_cast<double>(total_latency.load()) / comp
                    : 0.0;

            // Clear terminal
            std::cout << "\033[2J\033[H";

            std::cout << CYAN
                      << "=========== TASK SCHEDULER DASHBOARD ===========\n"
                      << RESET;

            std::cout << GREEN
                      << "Queue Size        : "
                      << RESET << qsize << '\n';

            std::cout << GREEN
                      << "Completed Tasks   : "
                      << RESET << comp << '\n';

            std::cout << YELLOW
                      << "Throughput        : "
                      << RESET
                      << std::fixed
                      << std::setprecision(2)
                      << throughput
                      << " tasks/sec\n";

            std::cout << YELLOW
                      << "Avg Latency       : "
                      << RESET
                      << avg_latency
                      << " ms\n";

            std::cout << CYAN
                      << "Threads           : "
                      << RESET
                      << thread_count
                      << '\n';

            std::cout << CYAN
                      << "================================================\n"
                      << RESET;

            std::cout.flush();
        }
    }
};
