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

    bool is_cancelled = false;   // ✅ instead of atomic

    Task(int id_, std::string name_, int priority_, std::function<void()> f)
        : id(id_), name(name_), base_priority(priority_),
          effective_priority(priority_), func(f) {
        enqueue_time = std::chrono::steady_clock::now();
    }
};
