#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool pool(4);

    for (int i = 0; i < 30; i++) {
        pool.submit(Task(i, "Task" + std::to_string(i), i % 3, [i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500 + i * 200));
            std::cout << "Executing Task " << i << "\n";
        }));
    }

    // Cancel a task (demo)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    pool.cancelTask(5);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    pool.shutdown();

    return 0;
}
