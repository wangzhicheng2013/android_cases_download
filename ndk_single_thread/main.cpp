#include <signal.h>
#include "download_task.hpp"
static void signal_handler(int sig) {
    global_exit_flag = true;
}
void process_tasks() {
    DIR *dp = opendir(TASKS_DIR);
    if (nullptr == dp) {
        printf("can not open dir:%s\n", TASKS_DIR);
        return;
    }
    struct dirent *ptr = nullptr;
    char path[1024] = "";
    download_task task;
    while ((ptr = readdir(dp)) != nullptr) {
        if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, "..")) {
            continue;
        }
        snprintf(path, sizeof(path), "%s%s", TASKS_DIR, ptr->d_name);
        if (false == file_is_stready(path)) {
            printf("%s is not ready!\n", path);
            continue;
        }
        printf("scan task file:%s\n", path);
        task.task_path = path;
        if (true == task.get_task_list()) {
            task.process();
        }
        remove(path);
        printf("%s has been deleted!\n", path);
    }
    closedir(dp);
}
int main() {
    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);
    if ((false == check_dir(CASES_DIR)) || (false == check_dir(TASKS_DIR))) {
        return -1;
    }
    printf("input max keep cases count in /ota/cases/:");
    scanf("%d", &max_keep_cases_count);
    if (max_keep_cases_count <= 0 || max_keep_cases_count > 1000) {
        printf("%d should be between 0 and 1000!", max_keep_cases_count);
        return -2;
    }
    while (false == global_exit_flag) {
        process_tasks();
        sleep(3);
    }

    return 0;
}