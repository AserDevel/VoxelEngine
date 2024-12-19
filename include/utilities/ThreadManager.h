#pragma once

#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadManager {
public:
    ThreadManager(size_t numThreads);
    ~ThreadManager();

    // Add a new task to the queue
    void addTask(const std::function<void()>& task);

    // Gracefully shut down the thread pool
    void shutdown();

private:
    // Worker threads
    std::vector<std::thread> threads;

    // Task queue
    std::queue<std::function<void()>> taskQueue;

    // Synchronization primitives
    std::mutex queueMutex;
    std::condition_variable condition;

    // Atomic flag to stop threads
    std::atomic<bool> stop;

    // Worker function
    void workerThread();
};
