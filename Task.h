#pragma once

#include <functional>
#include <string>
#include <chrono>
#include <atomic>
#include <utility>

class Task {
public:
    int id;
    std::string name;

    int base_priority;
    int effective_priority;

    std::chrono::steady_clock::time_point enqueue_time;

    std::function<void()> func;

    std::atomic<bool> cancelled;

    Task(int id,
         std::string name,
         int priority,
         std::function<void()> job)
        : id(id),
          name(std::move(name)),
          base_priority(priority),
          effective_priority(priority),
          enqueue_time(std::chrono::steady_clock::now()),
          func(std::move(job)),
          cancelled(false) {}

    // Move Constructor
    Task(Task&& other) noexcept
        : id(other.id),
          name(std::move(other.name)),
          base_priority(other.base_priority),
          effective_priority(other.effective_priority),
          enqueue_time(other.enqueue_time),
          func(std::move(other.func)),
          cancelled(other.cancelled.load()) {}

    // Move Assignment
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            id = other.id;
            name = std::move(other.name);
            base_priority = other.base_priority;
            effective_priority = other.effective_priority;
            enqueue_time = other.enqueue_time;
            func = std::move(other.func);
            cancelled.store(other.cancelled.load());
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
};
