#include "utilities/ThreadManager.h"
#include <iostream>

ThreadManager::ThreadManager(size_t numThreads) : stop(false) {
    // Create worker threads
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(&ThreadManager::workerThread, this);
    }
}

ThreadManager::~ThreadManager() {
    shutdown();
}

void ThreadManager::addTask(const std::function<void()>& task) {
    {
        // Lock the task queue
        std::unique_lock<std::mutex> lock(queueMutex);
        taskQueue.push(task);
    }
    // Notify one thread that a task is available
    condition.notify_one();
}

void ThreadManager::shutdown() {
    // Set the stop flag
    stop.store(true);
    // Notify all threads to wake up
    condition.notify_all();

    // Join all threads
    for (std::thread& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadManager::workerThread() {
    while (true) {
        std::function<void()> task;

        {
            // Lock the queue to retrieve a task
            std::unique_lock<std::mutex> lock(queueMutex);

            // Wait until there is a task or the thread manager is stopping
            condition.wait(lock, [this]() { return !taskQueue.empty() || stop.load(); });

            // Exit if stopping and no more tasks are left
            if (stop.load() && taskQueue.empty()) {
                return;
            }

            // Get the next task
            task = taskQueue.front();
            taskQueue.pop();
        }

        // Execute the task
        task();
    }
}
