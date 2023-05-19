#pragma once
#include "global_data_define.hpp"
#include "curl_download_file.hpp"
#include "task_stealed_thread_pool.hpp"
#include "public_tools.hpp"
extern task_stealed_thread_pool global_thread_pool;
extern std::mutex global_lock;
static size_t url_count = 0;
struct download_task {
    std::string task_path;
private:
    std::vector<std::string>task_list;
    static void download(const std::string &url) {
        printf("begin to download:%s\n", url.c_str());
        curl_utility::curl_download_file download_file_utility;
        if (false == download_file_utility.init(url.c_str())) {
            printf("%s init failed!\n", url.c_str());
            return;
        }
        std::string file_name;
        get_download_file_name(url, file_name);
        if (file_name.empty()) {
            printf("%s can not get file name!\n", url.c_str());
            return;
        }
        // saving files in the /ota directory can be slow
        if (false == download_file_utility.save_local_file(file_name.c_str())) {
            printf("download file:%s failed!\n", file_name.c_str());
            return;
        }
        printf("%s download ok!\n", file_name.c_str());
        std::lock_guard<std::mutex>lock(global_lock);
        move_file(file_name);
    }
    static void move_file(const std::string &file_name) {
        char cmd[256] = "";
        snprintf(cmd, sizeof(cmd), "%s%s", CASES_DIR, file_name.c_str());
        //printf("now mv file:%s\n", cmd);
        while ((false == global_exit_flag) && (get_files_count(CASES_DIR) >= max_keep_cases_count)) {
            printf("task count is up to the limit:%d\n", max_keep_cases_count);
            printf("waiting....!\n");
            sleep(SLEEP_TIME);
        }
        bool succ = true;
        if (rename(file_name.c_str(), cmd)) {   // ota is a different file system from others
            snprintf(cmd, sizeof(cmd), "mv -f %s %s", file_name.c_str(), CASES_DIR);
            if (system(cmd)) {
                printf("mv file failed!\n");
                succ = false;
            }
        }
        if (true == succ) {
            ++url_count;
        }
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
                continue;
            }
            char &ch = line[line.size() - 1];
            if ((13 == ch) || (10 == ch)) {      // CR or \n
                ch = 0;
            }
            task_list.push_back(line);
        }
        ifs.close();
        printf("get task list count:%zd from task:%s\n", task_list.size(), task_path.c_str());
        return true;
    }
    void process() {
        url_count = 0;
        for (auto &url : task_list) {
            while ((false == global_exit_flag) && (false == global_thread_pool.add_task(download, url))) {
                printf("add task to thread pool failed, try....!\n");
                sleep(SLEEP_TIME);
            }
            if (true == global_exit_flag) {
                break;
            }
        }
        while ((false == global_exit_flag) && (url_count < task_list.size())) {
            printf("%s have not been done!\n", task_path.c_str());
            sleep(SLEEP_TIME);
        }
        remove(task_path.c_str());
        printf("%s has been deleted!\n", task_path.c_str());
    }
};
