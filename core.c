#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "core.h"
#include "display.h"
#include "sort.h"

void init_file_list(FileList *list) {
    list->count = 0;
    list->capacity = 16;
    list->files = malloc(list->capacity * sizeof(FileInfo));
}

void add_file_info(FileList *list, FileInfo info) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->files = realloc(list->files, list->capacity * sizeof(FileInfo));
    }
    list->files[list->count++] = info;
}

void free_file_list(FileList *list) {
    free(list->files);
    list->files = NULL;
    list->count = 0;
    list->capacity = 0;
}

void process_path(const char *path, const LsOptions *opts) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        perror(path);
        return;
    }

    // Nếu path là file đơn lẻ hoặc có cờ -d
    if (!S_ISDIR(st.st_mode) || opts->directory_only) {
        FileInfo info;
        strncpy(info.name, path, sizeof(info.name) - 1);
        strncpy(info.full_path, path, sizeof(info.full_path) - 1);
        info.statbuf = st;
        info.stat_ok = 1;

        FileList list;
        init_file_list(&list);
        add_file_info(&list, info);
        display_files(&list, opts);
        free_file_list(&list);
        return;
    }

    // Nếu path là thư mục -> Mở và đọc danh sách các file con
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    FileList list;
    init_file_list(&list);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Xử lý cờ hiển thị file ẩn (-a / -A)
        if (entry->d_name[0] == '.') {
            if (!opts->show_all && !opts->show_almost_all) {
                continue; // Bỏ qua file ẩn nếu không bật -a hoặc -A
            }
            if (opts->show_almost_all && !opts->show_all) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue; // Bỏ qua . và .. nếu chỉ bật -A
                }
            }
        }

        FileInfo info;
        strncpy(info.name, entry->d_name, sizeof(info.name) - 1);
        
        // Tạo đường dẫn đầy đủ
        if (strcmp(path, "/") == 0) {
            snprintf(info.full_path, sizeof(info.full_path), "/%s", entry->d_name);
        } else {
            snprintf(info.full_path, sizeof(info.full_path), "%s/%s", path, entry->d_name);
        }

        if (lstat(info.full_path, &info.statbuf) == 0) {
            info.stat_ok = 1;
        } else {
            info.stat_ok = 0;
        }

        add_file_info(&list, info);
    }
    closedir(dir);
    sort_file_list(&list, opts);
    display_files(&list, opts);
    free_file_list(&list);
}