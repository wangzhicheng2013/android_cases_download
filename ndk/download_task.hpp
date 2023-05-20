#pragma once
#include "global_data_define.hpp"
#include "curl_download_file.hpp"
#include "task_stealed_thread_pool.hpp"
#include "public_tools.hpp"
#include "cond_queue.hpp"

extern bool global_exit_flag;
extern int max_keep_cases_count;
extern task_stealed_thread_pool global_thread_pool;
extern std::mutex global_lock;
extern cond_queue<std::string>global_queue; 
extern stat_data global_stat_data;

struct download_task {
    std::string task_path;
private:
    std::vector<std::string>task_list;
private:
    static void download(const std::string &url) {
        printf("begin to download:%s\n", url.c_str());
        curl_utility::curl_download_file download_file_utility;
        if (false == download_file_utility.init(url.c_str())) {
            ++(global_stat_data.failed_download_files_count);
            printf("url:%s init curl failed!\n", url.c_str());
            return;
        }
        std::string file_name;
        get_download_file_name(url, file_name);
        if (file_name.empty()) {
            ++(global_stat_data.failed_download_files_count);
            printf("%s can not get file name!\n", url.c_str());
            return;
        }
        // saving files in the /ota directory can be slow
        if (false == download_file_utility.save_local_file(file_name.c_str())) {
            ++(global_stat_data.failed_download_files_count);
            printf("download file:%s failed!\n", file_name.c_str());
            return;
        }
        printf("%s download ok!\n", file_name.c_str());
        global_queue.push(file_name);    // will block when queue is full
    }
public:
    bool get_task_list() {
        task_list.clear();
        std::ifstream ifs(task_path.c_str(), std::ios::in);
        if (!ifs || !ifs.is_open()) {
            printf("%s can not open!\n", task_path.c_str());
            return false;
        }
        std::string line;
        while (getline(ifs, line)) {
            if (line.empty()) {
                continue;
            }
            auto pos = line.find("http");       // check whether is url
            if (pos == std::string::npos) {
                printf("invalid line:%s\n", line.c_str());
                continue;
            }
            char &ch = line[line.size() - 1];
            if ((13 == ch) || (10 == ch)) {      // CR or \n
                ch = 0;
            }
            task_list.push_back(line);
        }
        ifs.close();
        printf("get task list count:%zd from task file:%s\n", task_list.size(), task_path.c_str());
        return task_list.size() > 0;
    }
    void process() {        // execute in scan file thread(single thread)
        for (auto &url : task_list) {
            while ((false == global_exit_flag) && (false == global_thread_pool.add_task(download, url))) {
                printf("add task to thread pool failed, try....!\n");
                THREAD_SLEEP(SLEEP_TIME);
            }
        }
        int url_count = task_list.size();
        while ((false == global_exit_flag) && (global_stat_data < url_count)) {
            printf("task:%s have not been done!\n", task_path.c_str());
            THREAD_SLEEP(SLEEP_TIME);
        }
        global_stat_data.show(); 
        global_stat_data.clear();
        remove(task_path.c_str());
        printf("task done, file:%s has been deleted!\n", task_path.c_str());
    }
};
