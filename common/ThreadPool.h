#pragma once
#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <thread>
#include <queue>
#include <atomic>
#include <sys/sysinfo.h>

class ThreadPool {
public:
    using task_t = std::function<void()>;

    ThreadPool(uint32_t block_count)
        : stop_(false) {
        uint32_t cpu_count = get_nprocs();
        uint32_t max_workers = std::min(cpu_count * 2, block_count);
        for (int32_t i = 0; i < max_workers; ++i) {
            std::string name = "worker_";
            name += std::to_string(i);
            workers_.emplace_back(&ThreadPool::workerThread, this, name);
        }
    }
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(pool_mutex_);
            stop_ = true;
        }
        pool_cv_.notify_all();
        for (auto &worker : workers_) {
            worker.join();
        }
    }

    void addTask(task_t task) {
        {
            std::lock_guard<std::mutex> lock(pool_mutex_);
            pending_tasks_.push(task);
        }
        pool_cv_.notify_one();
    }
private:
    void workerThread(std::string name="worker ") {
        pthread_setname_np(pthread_self(), name.c_str());
        while (!stop_ || !pending_tasks_.empty()) {
            task_t task = nullptr;
            {
                std::unique_lock<std::mutex> lock(pool_mutex_);
                pool_cv_.wait(lock, [this]{ return !pending_tasks_.empty() || stop_; });
                if (!pending_tasks_.empty()) {
                    task = pending_tasks_.front();
                    pending_tasks_.pop();
                }
            }
            if (task) {
                task();
            }
        }
    }

    std::mutex pool_mutex_;
    std::condition_variable pool_cv_;
    std::vector<std::thread> workers_;
    std::queue<task_t> pending_tasks_;
    std::atomic_bool stop_;
};
