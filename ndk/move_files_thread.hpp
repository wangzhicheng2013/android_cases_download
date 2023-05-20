#pragma once
#include "global_data_define.hpp"
#include "base_thread.hpp"
#include "cond_queue.hpp"
#include "public_tools.hpp"

extern bool global_exit_flag;
extern int max_keep_cases_count;
extern cond_queue<std::string>global_queue;
extern stat_data global_stat_data;

class move_files_thread final : public base_thread {
    virtual void process() override {
        int file_count = 0;
        int move_file_count = 0;
        while (false == global_exit_flag) {
            if (global_queue.empty()) {     // no task
                THREAD_SLEEP(SLEEP_TIME);
                continue;
            }
            file_count = get_files_count(CASES_DIR);
            if (file_count >= max_keep_cases_count) {
                printf("dir:%s task count is up to the limit:%d, waiting...!\n", CASES_DIR, max_keep_cases_count);
                THREAD_SLEEP(SLEEP_TIME);
                continue;
            }
            move_file_count = max_keep_cases_count - file_count;
            move_files(move_file_count);
        }
    }
private:
    void move_files(int move_file_count) {
        char buf[256] = "";
        std::string file_name;
        bool succ = true;
        for (int i = 0;(false == global_queue.empty()) && (i < move_file_count);i++) {
            global_queue.pop(file_name);    // will block if queue is empty
            snprintf(buf, sizeof(buf), "%s%s", CASES_DIR, file_name.c_str());
            printf("dest file:%s\n", buf);
            succ = true;
            if (rename(file_name.c_str(), buf)) {   // ota is a different file system from others
                snprintf(buf, sizeof(buf), "mv -f %s %s", file_name.c_str(), CASES_DIR);
                if (system(buf)) {
                    printf("mv file failed!\n");
                    succ = false;
                    ++(global_stat_data.failed_move_files_count);
                }
            }
            if (true == succ) {
                printf("file move ok!\n");
                ++(global_stat_data.succ_files_count);
            }
        }
    }
public:
    virtual ~move_files_thread() {
        printf("move_files_thread join!\n");
    }
};