#pragma once

#include <string>
#include <functional>
#include <chrono>
#include <atomic>
#include <utility>

struct Task {
    int id;
    std::string name;

    int base_priority;
    int effective_priority;

    std::chrono::steady_clock::time_point enqueue_time;

    std::function<void()> func;

    std::atomic<bool> is_cancelled{false};

    Task(int id_,
         std::string name_,
         int priority_,
         std::function<void()> f)
        : id(id_),
          name(std::move(name_)),
          base_priority(priority_),
          effective_priority(priority_),
          enqueue_time(std::chrono::steady_clock::now()),
          func(std::move(f)) {}

    // Move constructor
    Task(Task&& other) noexcept
        : id(other.id),
          name(std::move(other.name)),
          base_priority(other.base_priority),
          effective_priority(other.effective_priority),
          enqueue_time(other.enqueue_time),
          func(std::move(other.func)),
          is_cancelled(other.is_cancelled.load()) {}

    // Move assignment
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            id = other.id;
            name = std::move(other.name);
            base_priority = other.base_priority;
            effective_priority = other.effective_priority;
            enqueue_time = other.enqueue_time;
            func = std::move(other.func);
            is_cancelled.store(other.is_cancelled.load());
        }
        return *this;
    }

    // Disable copying
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
};
