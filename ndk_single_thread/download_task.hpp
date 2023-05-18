#pragma once
#include "global_data_define.hpp"
#include "curl_download_file.hpp"
#include "public_tools.hpp"
struct download_task {
    std::string task_path;
private:
    std::vector<std::string>task_list;
    static void check_files_count() {
        while ((false == global_exit_flag) && (get_files_count(CASES_DIR) >= max_keep_cases_count)) {
            printf("task count is up to the limit:%d\n", max_keep_cases_count);
            printf("waiting....!\n");
            sleep(3);
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
        char cmd[256] = "";
        for (auto &url : task_list) {
            check_files_count();
            if (true == global_exit_flag) {
                break;
            }
            printf("begin to download:%s\n", url.c_str());
            curl_utility::curl_download_file download_file_utility;
            if (false == download_file_utility.init(url.c_str())) {
                printf("%s init failed!\n", url.c_str());
                continue;
            }
            std::string file_name;
            get_download_file_name(url, file_name);
            if (file_name.empty()) {
                printf("%s can not get file name!\n", url.c_str());
                continue;
            }
            if (false == download_file_utility.save_local_file(file_name.c_str())) {
                printf("download file:%s failed!\n", file_name.c_str());
                continue;
            }
            printf("%s download ok!\n", file_name.c_str());
            snprintf(cmd, sizeof(cmd), "%s%s", CASES_DIR, file_name.c_str());
            printf("now to mv file:%s\n", cmd);
            if (rename(file_name.c_str(), cmd)) {   // move /data <=> /ota/ is very slow
                snprintf(cmd, sizeof(cmd), "mv -f %s %s", file_name.c_str(), CASES_DIR);
                if (system(cmd)) {
                    printf("mv file failed!\n");
                }
            }
        }
    }
};
