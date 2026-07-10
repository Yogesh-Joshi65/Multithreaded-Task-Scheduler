#include "ThreadPool.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {

    // 4 Worker Threads
    ThreadPool pool(4);

    std::cout
        << "\n=========== Task Scheduler Demo ===========\n\n";

    //---------------------------------------------------------
    // Submit 120 Normal Tasks
    //---------------------------------------------------------

    for (int i = 1; i <= 120; i++) {

        int priority = (i % 5) + 1;

        pool.submit(

            Task(

                i,

                "Task_" + std::to_string(i),

                priority,

                [i]() {

                    std::cout
                        << "Executing Task "
                        << i
                        << std::endl;

                    // Each task takes 2–4 seconds
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(
                            2000 + (i % 5) * 500));

                }

            )

        );
    }

    //---------------------------------------------------------
    // High Priority Task
    //---------------------------------------------------------

    pool.submit(

        Task(

            1000,

            "Critical_Task",

            100,

            []() {

                std::cout
                    << "\n******** HIGH PRIORITY TASK ********\n";

                std::this_thread::sleep_for(
                    std::chrono::seconds(5));

            }

        )

    );

    //---------------------------------------------------------
    // Exception Task
    //---------------------------------------------------------

    pool.submit(

        Task(

            1001,

            "Exception_Task",

            50,

            []() {

                std::this_thread::sleep_for(
                    std::chrono::seconds(2));

                throw std::runtime_error(
                    "Intentional Exception");

            }

        )

    );

    //---------------------------------------------------------
    // Cancel one task after 5 seconds
    //---------------------------------------------------------

    std::this_thread::sleep_for(
        std::chrono::seconds(5));

    std::cout
        << "\nCancelling Task 60...\n";

    pool.cancelTask(60);

    //---------------------------------------------------------
    // Keep scheduler running long enough
    // for monitor output.
    //---------------------------------------------------------

    std::this_thread::sleep_for(
        std::chrono::seconds(60));

    std::cout
        << "\nStopping Scheduler...\n";

    pool.shutdown();

    return 0;
}
