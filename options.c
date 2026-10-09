#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "options.h"

void init_options(LsOptions *opts) {
    opts->show_all = 0;
    opts->show_almost_all = 0;
    opts->long_format = 0;
    opts->numeric_uid_gid = 0;
    opts->classify = 0;
    opts->inode = 0;
    opts->blocks = 0;
    opts->human_readable = 0;
    opts->kilobyte = 0;
    opts->hide_non_print = 0;
    opts->raw_non_print = 0;
    opts->sort_none = 0;
    opts->sort_size = 0;
    opts->sort_time = 0;
    opts->reverse = 0;
    opts->time_status = 0;
    opts->time_access = 0;
    opts->directory_only = 0;
    opts->recursive = 0;
}

int parse_options(int argc, char *argv[], LsOptions *opts) {
    init_options(opts);
    int opt;

    // Chuỗi tham số cần bắt theo đúng Manual
    while ((opt = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (opt) {
            case 'A':
                opts->show_almost_all = 1;
                break;
            case 'a':
                opts->show_all = 1;
                break;
            case 'c':
                opts->time_status = 1;
                opts->time_access = 0; // -c ghi đè -u
                break;
            case 'd':
                opts->directory_only = 1;
                opts->recursive = 0;   // -d ghi đè -R
                break;
            case 'F':
                opts->classify = 1;
                break;
            case 'f':
                opts->sort_none = 1;
                break;
            case 'h':
                opts->human_readable = 1;
                opts->kilobyte = 0;    // -h ghi đè -k
                break;
            case 'i':
                opts->inode = 1;
                break;
            case 'k':
                opts->kilobyte = 1;
                opts->human_readable = 0; // -k ghi đè -h
                break;
            case 'l':
                opts->long_format = 1;
                opts->numeric_uid_gid = 0; // -l ghi đè -n
                break;
            case 'n':
                opts->numeric_uid_gid = 1;
                opts->long_format = 0;     // -n ghi đè -l
                break;
            case 'q':
                opts->hide_non_print = 1;
                opts->raw_non_print = 0;   // -q ghi đè -w
                break;
            case 'R':
                opts->recursive = 1;
                opts->directory_only = 0;  // -R ghi đè -d
                break;
            case 'r':
                opts->reverse = 1;
                break;
            case 'S':
                opts->sort_size = 1;
                break;
            case 's':
                opts->blocks = 1;
                break;
            case 't':
                opts->sort_time = 1;
                break;
            case 'u':
                opts->time_access = 1;
                opts->time_status = 0; // -u ghi đè -c
                break;
            case 'w':
                opts->raw_non_print = 1;
                opts->hide_non_print = 0;  // -w ghi đè -q
                break;
            default:
                // Nhập cờ không hợp lệ
                exit(EXIT_FAILURE);
        }
    }

    // Biến optind của getopt chứa vị trí bắt đầu của các tham số không phải là cờ (tên file/thư mục)
    return optind;
}