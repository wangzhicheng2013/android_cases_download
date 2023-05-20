#pragma once
#include <stdio.h>
#include <time.h>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>
#include <chrono>
#include "sync_deque.hpp"

extern bool global_exit_flag;
class worker_t;
using workers_ptr = std::shared_ptr<std::vector<std::shared_ptr<worker_t>>>;
using task_t = std::function<void(const std::string &)>;
class worker_t final {
public:
    explicit worker_t(workers_ptr workers, size_t work_num) : workers_(workers) {
        work_num_ = work_num;
        enabled_ = true;
        thd_ = std::thread(std::bind(&worker_t::execute, this));
    }
    inline bool assign(const task_t &task, const std::string &arg) {
        return queue_.push_front(std::make_pair(task, arg));
    }
    inline bool empty() {
        return true == queue_.empty();
    }
    void join() {
        enabled_ = false;
        if (thd_.joinable()) {
            printf("thread join.\n");
            thd_.join();
        }
    }
private:
    void execute_stealed_task() {
        int rand_select = rand() % work_num_;
        auto worker = workers_->at(rand_select);
        printf("%d work thread will be selected.\n", rand_select);
        std::pair<task_t, std::string>e;
        while (worker->queue_.pop_back(e)) {
            task_t &task = e.first;
            const std::string &arg = e.second;
            task(arg);
        }
    }
    void execute() {
        // thread_id_ = std::this_thread::get_id();
        while ((false == global_exit_flag) && enabled_) {
            if (work_num_ != workers_->size()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            std::pair<task_t, std::string>e;
			while (queue_.pop_front(e)) {
				task_t &task = e.first;
                const std::string &arg = e.second;
                task(arg);
			}
            bool no_task = std::all_of(workers_->begin(), workers_->end(), [] (std::shared_ptr<worker_t> worker) { return worker->empty(); });
            if (true == no_task) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            execute_stealed_task();
        }
    }
private:
    size_t work_num_;
    bool enabled_;
    std::thread thd_;
    workers_ptr workers_;
    std::thread::id thread_id_;
    sync_deque<std::pair<task_t, std::string>>queue_;
};
class task_stealed_thread_pool {
public:
    task_stealed_thread_pool(size_t thread_num = std::thread::hardware_concurrency()) {
        std::srand(time(nullptr));
        workers_ = std::make_shared<std::vector<std::shared_ptr<worker_t>>>();
        for (size_t i = 0;i < thread_num;i++) {
            auto worker = std::make_shared<worker_t>(workers_, thread_num);
            workers_->emplace_back(worker);
        }
    }
    ~task_stealed_thread_pool() {
        for (auto worker : *workers_) {
            worker->join();
        }
        workers_->clear();
        printf("task_stealed_thread_pool join.!\n");
    }
    inline bool add_task(const task_t &task, const std::string &arg) {
        auto worker = get_rand_worker();
        return worker->assign(task, arg);
    }
private:
    inline std::shared_ptr<worker_t> get_rand_worker() {
        static size_t size = workers_->size();
        return workers_->at(rand() % size);
    }
private:
    workers_ptr workers_;
};