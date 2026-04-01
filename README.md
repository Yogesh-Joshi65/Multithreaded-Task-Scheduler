# 🚀 Multithreaded Task Scheduler (C++)

A high-performance **multithreaded task scheduler** built in C++ that simulates real-world backend job processing systems (similar to thread pools used in production services).

---

## 📌 Overview

This project implements a **thread pool–based scheduler** where multiple worker threads execute tasks concurrently from a shared **priority queue**.

It demonstrates key systems concepts such as **concurrency control, scheduling policies, fairness, and observability**.

---

## ⚙️ Key Features

### 🧵 Thread Pool Execution

* Fixed-size pool of worker threads
* Efficient task processing without thread creation overhead

### 📊 Priority Scheduling with Aging

* Tasks are scheduled based on priority
* Aging mechanism increases priority over time
* Prevents starvation of low-priority tasks

### ❌ Task Cancellation

* Tasks can be cancelled before execution
* Workers safely skip cancelled tasks

### ⏱️ Timeout Handling

* Uses `std::future` to detect long-running tasks
* Prevents worker threads from blocking indefinitely

### 📈 Real-Time CLI Dashboard

Displays live system metrics:

* Queue size
* Completed tasks
* Throughput (tasks/sec)
* Average latency
* Active threads

---

## 🏗️ System Architecture

```
        +------------------+
        |   ThreadPool     |
        +------------------+
                 |
        +------------------+
        |   Task Queue     |  (Priority + Aging)
        +------------------+
           |          |
     +---------+  +---------+
     | Worker  |  | Worker  |  ... (N threads)
     +---------+  +---------+
           |
     Task Execution
           |
     +------------------+
     |   CLI Monitor    |
     +------------------+
```

---

## 🔄 Execution Flow

1. Tasks are submitted via `ThreadPool::submit()`
2. Tasks are inserted into a thread-safe priority queue
3. Worker threads wait using `condition_variable`
4. Highest-priority task is selected and executed
5. Aging mechanism periodically updates priorities
6. Monitor thread displays real-time metrics

---

## 📁 Project Structure

```
scheduler/
├── Task.h
├── TaskQueue.h
├── Worker.h
├── Monitor.h
├── ThreadPool.h
└── main.cpp
```

---

## 🖥️ Sample Output

```
=========== TASK SCHEDULER DASHBOARD ===========
Queue Size        : 6
Completed Tasks   : 4
Throughput        : 1.75 tasks/sec
Avg Latency       : 1300 ms
Active Threads    : 4
================================================

[Worker] Executing Task 2 | Base: 10 | Effective: 10
[Worker] Executing Task 7 | Base: 1  | Effective: 9
Task 8 timed out
```

---

## 🧠 Concepts Demonstrated

* Multithreading (`std::thread`)
* Synchronization (`mutex`, `condition_variable`)
* Producer–Consumer pattern
* Thread pool design
* Priority queue (heap-based scheduling)
* Starvation and aging
* Task lifecycle management
* Basic observability and metrics

---

## ⚖️ Design Decisions & Trade-offs

| Component    | Decision                   | Reason                                       |
| ------------ | -------------------------- | -------------------------------------------- |
| Queue        | Centralized priority queue | Simpler design, easier to reason             |
| Cancellation | Flag-based                 | Safe, avoids killing threads                 |
| Timeout      | Detection-based            | Prevents blocking without unsafe termination |
| Scheduling   | Priority + Aging           | Balances urgency and fairness                |

---

## 🚀 How to Run

### 🔹 Compile

```bash
g++ -std=c++17 -pthread main.cpp -o scheduler
```

### 🔹 Run

```bash
./scheduler
```

---

## 📈 Possible Improvements

* Work-stealing queues (reduce contention)
* Lock-free data structures
* Bounded queue + backpressure
* Persistent job storage
* Distributed scheduling (Kafka / SQS style)

---

## 🎯 Learning Outcomes

This project helped in understanding:

* Real-world concurrency challenges
* Efficient thread management
* Scheduling strategies in backend systems
* Designing observable systems with runtime metrics

---

## 👨‍💻 Author

**Yogesh Joshi**
C++ | Backend | Systems Programming

---
