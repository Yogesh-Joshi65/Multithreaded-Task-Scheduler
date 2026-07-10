#include "ThreadPool.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {

    ThreadPool pool(4);

    std::cout << "\n=========== Task Scheduler Demo ===========\n\n";

    // Normal Tasks
    for (int i = 1; i <= 50; i++) {

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

                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(
                            500 + (i % 5) * 200));

                }

            )

        );
    }

    //---------------------------------------------------------

    // High Priority Task

    pool.submit(

        Task(

            100,

            "Critical_Task",

            100,

            []() {

                std::cout
                    << "\n******** HIGH PRIORITY TASK ********\n";

                std::this_thread::sleep_for(
                    std::chrono::seconds(2));

            }

        )

    );

    //---------------------------------------------------------

    // Exception Task

    pool.submit(

        Task(

            101,

            "Exception_Task",

            50,

            []() {

                throw std::runtime_error(
                    "Intentional Exception");

            }

        )

    );

    //---------------------------------------------------------

    std::this_thread::sleep_for(
        std::chrono::seconds(2));

    std::cout
        << "\nCancelling Task 25...\n";

    pool.cancelTask(25);

    //---------------------------------------------------------

    std::this_thread::sleep_for(
        std::chrono::seconds(20));

    std::cout
        << "\nStopping Scheduler...\n";

    pool.shutdown();

    return 0;
}
