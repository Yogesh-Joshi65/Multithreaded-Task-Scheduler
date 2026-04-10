#pragma once
#include <functional>
#include <chrono>
#include <string>
#include <atomic>

struct Task {
    int id;
    std::string name;

    int base_priority;
    int effective_priority;

    std::chrono::steady_clock::time_point enqueue_time;

    std::function<void()> func;

    std::atomic<bool> is_cancelled{false};

    Task(int id_, std::string name_, int priority_, std::function<void()> f)
        : id(id_),
          name(std::move(name_)),
          base_priority(priority_),
          effective_priority(priority_),
          func(std::move(f)),
          enqueue_time(std::chrono::steady_clock::now()) {}

    // Move only (better for performance & safety)
    Task(Task&&) = default;
    Task& operator=(Task&&) = default;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
};
