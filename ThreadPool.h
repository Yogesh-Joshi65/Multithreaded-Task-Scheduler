class ThreadPool {
private:
    TaskQueue queue;
    std::vector<std::thread> workers;
    std::thread aging_thread;
    std::thread monitor_thread;

    std::atomic<bool> stop_flag{false};

    std::atomic<int> completed{0};
    std::atomic<long long> total_latency{0};

public:
    ThreadPool(int n) {
        for (int i = 0; i < n; i++) {
            workers.emplace_back(
                Worker(queue, std::ref(stop_flag), completed, total_latency));
        }

        aging_thread = std::thread([this]() {
            while (!stop_flag) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                queue.apply_aging();
            }
        });

        monitor_thread = std::thread(
            Monitor(queue, completed, total_latency, stop_flag, n));
    }

    void submit(Task task) {
        queue.push(task);
    }

    void shutdown() {
        stop_flag = true;
        queue.notify_all();

        for (auto &t : workers) t.join();
        aging_thread.join();
        monitor_thread.join();
    }

    ~ThreadPool() {
        if (!stop_flag) shutdown();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};
