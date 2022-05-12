#pragma once
#include <stdint.h>
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "inc/ExecContext.h"

class CUResource {
public:
    CUResource() {
        m_max_bar_slots = 100;
        m_free_bar_slots = 800;
        m_max_cta_per_cu = 100;
    }


    static CUResource* GetCUResource(uint32_t cu_id) {
        static std::map<uint32_t, CUResource*> table;
        if (table.find(cu_id) == table.end()) {
            table.insert(std::make_pair(cu_id, new CUResource));
        }
        return table[cu_id];
    }

    bool allocRunningBlock(uint32_t bar_count) {
        std::unique_lock<std::mutex> lock(scheduler_mutex_);
        scheduler_cv_.wait(lock, [this, &bar_count]{
                return (m_running_block_count < m_max_cta_per_cu) &&
                        (bar_count < m_free_bar_slots)
                        ;
                });
        m_running_block_count++;
    }

    bool freeRunningBlock() {
        std::lock_guard<std::mutex> lock(scheduler_mutex_);
        m_running_block_count--;
        scheduler_cv_.notify_one();
    }

    bool allocBarSlot(uint32_t bar_count) {
        std::lock_guard<std::mutex> lock(scheduler_mutex_);
        if (bar_count > m_free_bar_slots) return false;
        m_free_bar_slots -= bar_count;
        return true;
    }

    void freeBarSlot(uint32_t bar_count) {
        std::lock_guard<std::mutex> lock(scheduler_mutex_);
        m_free_bar_slots += bar_count;
    }

private:
    uint32_t m_max_bar_slots;
    uint32_t m_free_bar_slots;
    uint32_t m_max_cta_per_cu {16};
    std::mutex scheduler_mutex_;
    std::condition_variable scheduler_cv_;
    std::atomic_int m_running_block_count {0};
};
