#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <utility>

enum class TaskStatus {
    Waiting,
    Running,
    Completed,
    Cancelled,
    Failed
};

class Task {
public:
    int id;
    std::string name;

    int basePriority;
    int effectivePriority;

    std::chrono::steady_clock::time_point enqueueTime;

    std::function<void()> work;

    std::atomic<bool> cancelled{false};
    std::atomic<TaskStatus> status{TaskStatus::Waiting};

    Task(
        int id,
        std::string name,
        int priority,
        std::function<void()> job)
        : id(id),
          name(std::move(name)),
          basePriority(priority),
          effectivePriority(priority),
          enqueueTime(std::chrono::steady_clock::now()),
          work(std::move(job)) {}

    // Move Constructor
    Task(Task&& other) noexcept
        : id(other.id),
          name(std::move(other.name)),
          basePriority(other.basePriority),
          effectivePriority(other.effectivePriority),
          enqueueTime(other.enqueueTime),
          work(std::move(other.work)),
          cancelled(other.cancelled.load()),
          status(other.status.load()) {}

    // Move Assignment
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {

            id = other.id;
            name = std::move(other.name);

            basePriority = other.basePriority;
            effectivePriority = other.effectivePriority;

            enqueueTime = other.enqueueTime;

            work = std::move(other.work);

            cancelled.store(other.cancelled.load());

            status.store(other.status.load());
        }

        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    void cancel() {
        cancelled.store(true);
        status.store(TaskStatus::Cancelled);
    }

    bool isCancelled() const {
        return cancelled.load();
    }

    void markRunning() {
        status.store(TaskStatus::Running);
    }

    void markCompleted() {
        status.store(TaskStatus::Completed);
    }

    void markFailed() {
        status.store(TaskStatus::Failed);
    }
};
