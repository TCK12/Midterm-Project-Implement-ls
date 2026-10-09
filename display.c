#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "display.h"

// Hàm chuyển đổi kích thước file theo -h hoặc -k
static void format_size(off_t size, char *buf, size_t buf_size, const LsOptions *opts) {
    if (opts->human_readable) {
        const char *units[] = {"B", "K", "M", "G", "T"};
        double s = (double)size;
        int unit_idx = 0;
        while (s >= 1024 && unit_idx < 4) {
            s /= 1024;
            unit_idx++;
        }
        if (unit_idx == 0) {
            snprintf(buf, buf_size, "%4lldB", (long long)size);
        } else {
            snprintf(buf, buf_size, "%4.1f%s", s, units[unit_idx]);
        }
    } else if (opts->kilobyte) {
        long long kb = (size + 1023) / 1024; // Làm tròn lên KB
        snprintf(buf, buf_size, "%lldK", kb);
    } else {
        snprintf(buf, buf_size, "%lld", (long long)size);
    }
}

// In tên file xử lý cờ -q (thay ký tự không in được bằng '?') hoặc -w
static void print_filename(const char *name, const LsOptions *opts) {
    for (size_t i = 0; i < strlen(name); i++) {
        unsigned char c = (unsigned char)name[i];
        if (opts->hide_non_print || (!opts->raw_non_print && isatty(STDOUT_FILENO) && !isprint(c))) {
            putchar('?');
        } else {
            putchar(c);
        }
    }
}

static void get_mode_string(mode_t mode, char *str) {
    if (S_ISDIR(mode))       str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_ISUID) ? ((mode & S_IXUSR) ? 's' : 'S') : ((mode & S_IXUSR) ? 'x' : '-');

    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_ISGID) ? ((mode & S_IXGRP) ? 's' : 'S') : ((mode & S_IXGRP) ? 'x' : '-');

    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_ISVTX) ? ((mode & S_IXOTH) ? 't' : 'T') : ((mode & S_IXOTH) ? 'x' : '-');

    str[10] = '\0';
}

static void get_time_string(const struct stat *st, const LsOptions *opts, char *str, size_t maxsize) {
    time_t t;
    if (opts->time_status)       t = st->st_ctime;
    else if (opts->time_access)  t = st->st_atime;
    else                         t = st->st_mtime;

    struct tm *tm_info = localtime(&t);
    strftime(str, maxsize, "%b %e %H:%M", tm_info);
}

static void print_long_entry(const FileInfo *file, const LsOptions *opts) {
    char mode_str[11];
    get_mode_string(file->statbuf.st_mode, mode_str);

    char user_str[32], group_str[32];
    if (opts->numeric_uid_gid) {
        snprintf(user_str, sizeof(user_str), "%u", file->statbuf.st_uid);
        snprintf(group_str, sizeof(group_str), "%u", file->statbuf.st_gid);
    } else {
        struct passwd *pw = getpwuid(file->statbuf.st_uid);
        if (pw) strncpy(user_str, pw->pw_name, sizeof(user_str) - 1);
        else snprintf(user_str, sizeof(user_str), "%u", file->statbuf.st_uid);

        struct group *gr = getgrgid(file->statbuf.st_gid);
        if (gr) strncpy(group_str, gr->gr_name, sizeof(group_str) - 1);
        else snprintf(group_str, sizeof(group_str), "%u", file->statbuf.st_gid);
    }

    char time_str[64];
    get_time_string(&file->statbuf, opts, time_str, sizeof(time_str));

    char size_str[32];
    format_size(file->statbuf.st_size, size_str, sizeof(size_str), opts);

    printf("%s %2lu %s %s %8s %s ",
           mode_str,
           (unsigned long)file->statbuf.st_nlink,
           user_str,
           group_str,
           size_str,
           time_str);

    print_filename(file->name, opts);

    if (S_ISLNK(file->statbuf.st_mode)) {
        char link_target[1024];
        ssize_t len = readlink(file->full_path, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf(" -> ");
            print_filename(link_target, opts);
        }
    }

    if (opts->classify) {
        mode_t m = file->statbuf.st_mode;
        if (S_ISDIR(m)) printf("/");
        else if (S_ISLNK(m)) printf("@");
        else if (S_ISSOCK(m)) printf("=");
        else if (S_ISFIFO(m)) printf("|");
        else if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) printf("*");
    }

    printf("\n");
}

void display_files(const FileList *list, const LsOptions *opts) {
    if (list->count == 0) return;

    if (opts->long_format || opts->numeric_uid_gid) {
        long long total_blocks = 0;
        for (int i = 0; i < list->count; i++) {
            total_blocks += list->files[i].statbuf.st_blocks;
        }
        printf("total %lld\n", total_blocks);

        for (int i = 0; i < list->count; i++) {
            print_long_entry(&list->files[i], opts);
        }
    } else {
        for (int i = 0; i < list->count; i++) {
            if (opts->inode) printf("%lu ", (unsigned long)list->files[i].statbuf.st_ino);
            if (opts->blocks) printf("%lld ", (long long)list->files[i].statbuf.st_blocks);

            print_filename(list->files[i].name, opts);

            if (opts->classify) {
                mode_t m = list->files[i].statbuf.st_mode;
                if (S_ISDIR(m)) printf("/");
                else if (S_ISLNK(m)) printf("@");
                else if (S_ISSOCK(m)) printf("=");
                else if (S_ISFIFO(m)) printf("|");
                else if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) printf("*");
            }
            printf("\n");
        }
    }
}