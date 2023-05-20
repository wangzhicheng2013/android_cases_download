#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>
template <class T>
class cond_queue {
public:
    cond_queue() : size_(1024) {
    }
    inline void set_size(size_t sz) {
        size_ = sz;
    }
    void push(const T &e) {
        std::unique_lock<std::mutex>lock(mutex_);
        while (queue_.size() >= size_) {
            push_cond_.wait(lock); 
        }
        queue_.push_back(e);
        lock.unlock();
        pop_cond_.notify_all();
    }
    void pop(T &e) {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            pop_cond_.wait(lock);   // block here and wait cond unlock 
        }
        e = queue_.front();
        queue_.pop_front();
        lock.unlock();
        push_cond_.notify_all();
    }
    inline bool empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }
private:
    size_t size_;
    std::deque<T>               queue_;
    std::mutex                  mutex_;
    std::condition_variable     push_cond_;
    std::condition_variable     pop_cond_;
};