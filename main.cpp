#include "ThreadPool.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

int main() {

    ThreadPool pool(4);

    std::cout << "Submitting tasks...\n";

    // Submit normal tasks
    for (int i = 1; i <= 50; i++) {

        pool.submit(
            Task(
                i,
                "Task_" + std::to_string(i),
                i % 5,
                [i]() {

                    std::cout
                        << "Executing Task "
                        << i
                        << '\n';

                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(
                            200 + (i % 5) * 100));

                }));
    }

    // High Priority Task
    pool.submit(

        Task(

            100,

            "HighPriority",

            100,

            []() {

                std::cout
                    << "\n***** HIGH PRIORITY TASK *****\n";

            }));
    

    // Exception Task
    pool.submit(

        Task(

            101,

            "Exception",

            50,

            []() {

                throw std::runtime_error(
                    "Intentional Exception");

            }));


    std::this_thread::sleep_for(
        std::chrono::seconds(1));

    std::cout
        << "\nCancelling Task 20\n";

    pool.cancelTask(20);


    std::this_thread::sleep_for(
        std::chrono::seconds(10));


    std::cout
        << "\nStopping Scheduler...\n";

    pool.shutdown();

    std::cout
        << "Scheduler stopped successfully.\n";

    return 0;
}
