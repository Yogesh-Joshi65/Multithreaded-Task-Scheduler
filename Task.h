#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <utility>

// Represents the current execution state of a task.
enum class TaskStatus {
    Waiting,    // Task is in queue and waiting for execution
    Running,    // Worker thread is executing the task
    Completed,  // Task finished successfully
    Cancelled,  // Task was cancelled before execution
    Failed      // Exception occurred during execution
};

class Task {
public:

    // ---------------- Basic Task Information ----------------

    // Unique identifier for each task
    int id;

    // Human-readable task name
    std::string name;

    // ---------------- Priority Information ----------------

    // Original priority assigned by the user.
    // Never changes.
    int basePriority;

    // Current priority used by scheduler.
    // Can increase due to priority aging.
    int effectivePriority;

    // Time when task entered the scheduler queue.
    // Used for priority aging calculations.
    std::chrono::steady_clock::time_point enqueueTime;

    // Actual work that worker threads execute.
    std::function<void()> work;

    // ---------------- Thread-Safe State ----------------

    // Indicates whether task has been cancelled.
    // Atomic because multiple threads may read/write it.
    std::atomic<bool> cancelled{false};

    // Current lifecycle state of the task.
    // Atomic avoids race conditions between worker threads.
    std::atomic<TaskStatus> status{TaskStatus::Waiting};

    // ----------------------------------------------------
    // Constructor
    // Initializes all task information.
    // enqueueTime stores current time for aging algorithm.
    // ----------------------------------------------------
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

    // ----------------------------------------------------
    // Move Constructor
    //
    // Allows ownership transfer without copying.
    // std::function and std::string are efficiently moved.
    // Atomic variables cannot be moved directly,
    // so their values are copied using load().
    // ----------------------------------------------------
    Task(Task&& other) noexcept
        : id(other.id),
          name(std::move(other.name)),
          basePriority(other.basePriority),
          effectivePriority(other.effectivePriority),
          enqueueTime(other.enqueueTime),
          work(std::move(other.work)),
          cancelled(other.cancelled.load()),
          status(other.status.load()) {}

    // ----------------------------------------------------
    // Move Assignment Operator
    //
    // Transfers ownership from another Task.
    // Self-assignment check prevents unnecessary work.
    // Atomic members are copied using load()/store().
    // ----------------------------------------------------
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

    // Disable copying because std::atomic objects
    // are non-copyable and each task should have
    // unique ownership.
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    // ----------------------------------------------------
    // Marks the task as cancelled.
    // Worker threads check this flag before execution.
    // ----------------------------------------------------
    void cancel() {
        cancelled.store(true);
        status.store(TaskStatus::Cancelled);
    }

    // Returns true if task has been cancelled.
    bool isCancelled() const {
        return cancelled.load();
    }

    // Called just before executing task.
    void markRunning() {
        status.store(TaskStatus::Running);
    }

    // Called after successful execution.
    void markCompleted() {
        status.store(TaskStatus::Completed);
    }

    // Called if task throws an exception.
    void markFailed() {
        status.store(TaskStatus::Failed);
    }
};
