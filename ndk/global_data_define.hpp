#pragma once
#include <atomic>
#include <stdio.h>
#include <thread>
#include <chrono>
static const char* CASES_DIR = "/ota/cases/";
static const char* TASKS_DIR = "/ota/tasks/";
static const int SLEEP_TIME = 1;

struct stat_data {
    std::atomic<int>succ_files_count;
    std::atomic<int>failed_download_files_count;
    std::atomic<int>failed_move_files_count;

    stat_data() {
        succ_files_count = 0;
        failed_download_files_count = 0;
        failed_move_files_count = 0;
    }
    inline bool operator < (int count) {
        return succ_files_count + failed_download_files_count + failed_move_files_count < count;
    }
    inline void clear() {
        succ_files_count = 0;
        failed_download_files_count = 0;
        failed_move_files_count = 0;
    }
    inline void show() {
        printf("succ:%d download failed:%d, move failed:%d\n", succ_files_count.load(),
                                                               failed_download_files_count.load(),
                                                               failed_move_files_count.load());
    }
};

#define THREAD_SLEEP(duration) std::this_thread::sleep_for(std::chrono::seconds(duration));