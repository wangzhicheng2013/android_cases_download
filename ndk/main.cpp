#include <signal.h>
#include "scan_files_thread.hpp"
#include "move_files_thread.hpp"

int max_keep_cases_count = 0;
bool global_exit_flag = false;

task_stealed_thread_pool global_thread_pool;
std::mutex global_lock;
cond_queue<std::string>global_queue;        // stored downloaded files name
stat_data global_stat_data;
static void signal_handler(int sig) {
    global_exit_flag = true;
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
    global_queue.set_size(max_keep_cases_count);
    scan_files_thread scan_thread;
    move_files_thread move_thread;
    scan_thread.run();
    move_thread.run();
    scan_thread.join();
    move_thread.join();

    return 0;
}