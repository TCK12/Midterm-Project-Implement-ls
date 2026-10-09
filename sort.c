#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "sort.h"

static const LsOptions *g_opts = NULL;

static int compare_files(const void *a, const void *b) {
    const FileInfo *f1 = (const FileInfo *)a;
    const FileInfo *f2 = (const FileInfo *)b;

    int res = 0;

    // 1. Sắp xếp theo dung lượng (-S)
    if (g_opts->sort_size) {
        if (f1->statbuf.st_size < f2->statbuf.st_size) {
            res = 1;  // File lớn hơn lên đầu
        } else if (f1->statbuf.st_size > f2->statbuf.st_size) {
            res = -1;
        } else {
            res = strcmp(f1->name, f2->name); // Bằng dung lượng thì so theo tên
        }
    } 
    // 2. Sắp xếp theo thời gian (-t)
    else if (g_opts->sort_time) {
        time_t t1, t2;

        if (g_opts->time_status) {       // -c: Status change time
            t1 = f1->statbuf.st_ctime;
            t2 = f2->statbuf.st_ctime;
        } else if (g_opts->time_access) { // -u: Access time
            t1 = f1->statbuf.st_atime;
            t2 = f2->statbuf.st_atime;
        } else {                          // Mặc định: Modification time
            t1 = f1->statbuf.st_mtime;
            t2 = f2->statbuf.st_mtime;
        }

        if (t1 < t2) {
            res = 1;  // Mới nhất (timestamp lớn hơn) xếp trước
        } else if (t1 > t2) {
            res = -1;
        } else {
            res = strcmp(f1->name, f2->name); // Bằng thời gian thì so theo tên
        }
    } 
    // 3. Mặc định: Sắp xếp theo tên (lexicographical order)
    else {
        res = strcmp(f1->name, f2->name);
    }

    // Đảo ngược kết quả nếu có cờ -r
    if (g_opts->reverse) {
        res = -res;
    }

    return res;
}

void sort_file_list(FileList *list, const LsOptions *opts) {
    // -f (sort_none): Không sắp xếp
    if (opts->sort_none || list->count <= 1) {
        return;
    }
    
    g_opts = opts;
    qsort(list->files, list->count, sizeof(FileInfo), compare_files);
}