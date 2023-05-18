#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <fstream>
#include <vector>
static bool dir_existed(const char *path) {
    if (!path) {
        return false;
    }
    struct stat dir_stat;
    if (stat(path, &dir_stat) < 0) {
        return false;
    }
    return S_ISDIR(dir_stat.st_mode);
}
static bool create_dir(const char *path) {
    if (!path) {
        return false;
    }
    char cmd[128] = "";
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    int ret = system(cmd);
    usleep(100);
    return (0 == ret) && dir_existed(path);
}
static inline long get_last_change_time_for_file(const char *path) {
    struct stat fs;
    if (lstat(path, &fs) < 0) {
        return -1;
    }
    return fs.st_ctime;
}
static inline bool file_is_stready(const char *path) {
    long t = get_last_change_time_for_file(path);
    if (t < 0) {
        return false;
    }
    return (time(nullptr) - t) >= 3;
}
static bool check_dir(const char *dir_path) {
    if (false == dir_existed(dir_path)) {
        printf("%s does nott existed, now to create!\n", dir_path);
        if (true == create_dir(dir_path)) {
            printf("%s create ok!\n", dir_path);
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}
static int get_files_count(const char *path) {
    DIR *dp = opendir(path);
    if (nullptr == dp) {
        printf("can not open dir:%s\n", path);
        return -1;
    }
    struct dirent *ptr = nullptr;
    int count = 0;
    while ((ptr = readdir(dp)) != nullptr) {
        if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, "..")) {
            continue;
        }
        ++count;
    }
    closedir(dp);
    printf("read %d files in %s!\n", count, path);
    return count;
}
static void get_download_file_name(const std::string &url, std::string &file_name) {
    std::string::size_type pos = url.rfind("/");
    if (pos != std::string::npos) {
        file_name.assign(url, pos + 1, url.size() - pos - 1);
    }
}