# 🚀 Multithreaded Task Scheduler (C++) 

A high-performance **multithreaded task scheduler** built in C++ that simulates real-world backend job execution systems (similar to thread pools used in production services).

---

## 📌 Overview

This project implements a **thread pool–based scheduler** where multiple worker threads execute tasks concurrently from a shared **priority queue**.

It demonstrates core systems concepts such as **concurrency control, scheduling policies, fairness, and runtime observability**.

---

## ⚙️ Key Features

### 🧵 Thread Pool Execution

* Fixed-size pool of worker threads
* Eliminates overhead of frequent thread creation/destruction
* Efficient concurrent task execution

---

### 📊 Priority Scheduling with Aging

* Tasks are scheduled based on priority
* Aging mechanism gradually increases priority over time
* Prevents starvation of low-priority tasks

---

### ❌ Task Cancellation

* Tasks can be cancelled before execution
* Workers safely skip cancelled tasks
* Uses flag-based cancellation (safe and non-intrusive)

---

### ⏱️ Timeout Detection (Best-Effort)

* Detects long-running tasks using `std::future::wait_for`
* Logs timeout events for observability
* ⚠️ Does not forcibly terminate tasks (C++ does not support safe thread killing)

---

### 📈 Real-Time CLI Monitoring

Displays live system metrics:

* Queue size
* Completed tasks
* Throughput (tasks/sec)
* Average latency
* Configured worker threads

---

## 🏗️ System Architecture

```
        +------------------+
        |   ThreadPool     |
        +------------------+
                 |
        +--------------------------+
        |   Task Queue (Priority)  |
        |   + Aging Mechanism      |
        +--------------------------+
           |          |
     +---------+  +---------+
     | Worker  |  | Worker  |   ... (N threads)
     +---------+  +---------+
           |
     Task Execution
           |
     +------------------+
     |     Monitor      |
     +------------------+
```

---

## 🔄 Execution Flow

1. Tasks are submitted via `ThreadPool::submit()`
2. Tasks are inserted into a **thread-safe priority queue**
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
Throughput        : 2.00 tasks/sec
Avg Latency       : 1250.50 ms
Configured Threads: 4
================================================

[Worker-1] Executing Task 2 | Priority: 10
[Worker-3] Executing Task 7 | Priority: 9 (aged)
[Monitor] Task 8 exceeded timeout threshold
```

---

## 🧠 Concepts Demonstrated

* Multithreading (`std::thread`)
* Synchronization (`mutex`, `condition_variable`)
* Producer–Consumer pattern
* Thread pool design
* Priority scheduling (heap-based queue)
* Starvation prevention (aging)
* Task lifecycle management
* Runtime observability & metrics

---

## ⚡ Performance Characteristics

* Task insertion: **O(log n)** (priority queue)
* Task retrieval: **O(log n)**
* Concurrency model: **shared queue + worker threads**
* Potential bottleneck: centralized queue lock under high contention

---

## ⚖️ Design Decisions & Trade-offs

| Component    | Decision                   | Trade-off                                    |
| ------------ | -------------------------- | -------------------------------------------- |
| Queue        | Centralized priority queue | Simple, but can become contention bottleneck |
| Scheduling   | Priority + Aging           | Prevents starvation, adds periodic overhead  |
| Cancellation | Flag-based                 | Safe, but cannot stop running tasks          |
| Timeout      | Detection-only             | Observable, not enforceable                  |

---

## ⚠️ Limitations

* No preemptive task cancellation (C++ limitation)
* Timeout mechanism cannot terminate running tasks
* Centralized queue may limit scalability
* No backpressure for overload scenarios
* Monitoring uses console I/O (not production-grade logging)

---

## 🌍 Real-World Relevance

This system models components commonly used in production:

* Thread pools in web servers
* Background job processors (e.g., task queues)
* OS-level scheduling concepts (priority + fairness)
* Backend service execution pipelines

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
* Bounded queue with backpressure
* Cooperative task cancellation (cancellation tokens)
* Persistent job storage
* Distributed scheduling (Kafka / SQS style systems)

---

## 🎯 Learning Outcomes

* Deep understanding of concurrency and synchronization
* Designing efficient thread pools
* Implementing scheduling policies
* Handling real-world system trade-offs
* Building observable systems with runtime metrics

---

## 👨‍💻 Author
**Yogesh Joshi**

