#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool pool(4);

    std::cout << "Submitting tasks...\n";

    // Stress + priority mix
    for (int i = 0; i < 50; i++) {
        pool.submit(Task(i, "Task" + std::to_string(i), i % 5, [i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200 + i * 50));
            std::cout << "Executing Task " << i << "\n";
        }));
    }

    // High priority task
    pool.submit(Task(100, "HIGH_PRIORITY", 100, []() {
        std::cout << "🔥 High priority task executed\n";
    }));

    // Exception test
    pool.submit(Task(200, "ExceptionTask", 2, []() {
        throw std::runtime_error("Intentional failure");
    }));

    // Cancellation test
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Cancelling Task 5...\n";
    pool.cancelTask(5);

    // Wait for tasks (better than fixed sleep)
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Shutting down...\n";
    pool.shutdown();

    std::cout << "Shutdown complete.\n";
    return 0;
}
