#include "ThreadPool.h"

#include <cassert>

ThreadPool::ThreadPool(size_t threads) : 
    stop_(false), 
    activeTasks_(0) {
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
    }
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this] {
                        return stop_ || !taskQueue_.empty();
                        });
                    if (stop_ && taskQueue_.empty()) {
                        return;
                    }
                    task = std::move(taskQueue_.front());
                    taskQueue_.pop();
                    ++activeTasks_;
                }
                task();
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    --activeTasks_;
                    if (taskQueue_.empty() && activeTasks_ == 0) {
                        completionCondition_.notify_all();
                    }
                }
            }
            });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
        worker.join();
    }
}

void ThreadPool::PushTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stop_) {
            assert(false);
            return;
        }
        taskQueue_.emplace(task);
    }
    condition_.notify_one();
}

void ThreadPool::WaitForAll() {
    std::unique_lock<std::mutex> lock(mutex_);
    completionCondition_.wait(lock, [this] {
        return taskQueue_.empty() && activeTasks_ == 0;
        });
}
