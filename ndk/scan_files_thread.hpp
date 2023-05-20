#pragma once
#include <atomic>
#include "base_thread.hpp"
#include "download_task.hpp"

extern bool global_exit_flag;

class scan_files_thread final : public base_thread {
    virtual void process() override {
        DIR *dp = nullptr;
        struct dirent *ptr = nullptr;
        char path[256] = "";
        download_task task;
        while (false == global_exit_flag) {
            dp = opendir(TASKS_DIR);
            if (nullptr == dp) {
                printf("can not open dir:%s\n", TASKS_DIR);
                return;
            }
            while ((ptr = readdir(dp)) != nullptr) {
                if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, "..")) {
                    continue;
                }
                snprintf(path, sizeof(path), "%s%s", TASKS_DIR, ptr->d_name);
                if (false == file_is_stready(path)) {
                    printf("%s is not ready!\n", path);
                    continue;
                }
                printf("scan task file:%s and begin execute task!\n", path);
                task.task_path = path;
                if (true == task.get_task_list()) {
                    task.process();
                }
            }
            closedir(dp);
            THREAD_SLEEP(SLEEP_TIME);
        }
    }
public:
    virtual ~scan_files_thread() {
        printf("scan_files_thread join!\n");
    }
};