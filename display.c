#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "display.h"

// 1. Chuyển đổi mode (st_mode) thành chuỗi 10 ký tự quyền (vd: -rwxr-xr-x)
static void get_mode_string(mode_t mode, char *str) {
    // Loại file (Character 1)
    if (S_ISDIR(mode))       str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '-';

    // Quyền của Owner (rwx)
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        str[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        str[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    // Quyền của Group (rwx)
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        str[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        str[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    // Quyền của Other (rwx)
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) { // Sticky bit
        str[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        str[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    str[10] = '\0';
}

// 2. Định dạng ngày tháng chỉnh sửa gần nhất
static void get_time_string(const struct stat *st, const LsOptions *opts, char *str, size_t maxsize) {
    time_t t;
    if (opts->time_status) {
        t = st->st_ctime;
    } else if (opts->time_access) {
        t = st->st_atime;
    } else {
        t = st->st_mtime;
    }

    struct tm *tm_info = localtime(&t);
    // Định dạng: Tháng (3 chữ) Ngày Giờ:Phút (vd: Oct 27 14:30)
    strftime(str, maxsize, "%b %e %H:%M", tm_info);
}

// 3. In một dòng chi tiết theo định dạng ls -l
static void print_long_entry(const FileInfo *file, const LsOptions *opts) {
    char mode_str[11];
    get_mode_string(file->statbuf.st_mode, mode_str);

    // Lấy User name / Group name hoặc ID
    char user_str[32];
    char group_str[32];

    if (opts->numeric_uid_gid) {
        snprintf(user_str, sizeof(user_str), "%u", file->statbuf.st_uid);
        snprintf(group_str, sizeof(group_str), "%u", file->statbuf.st_gid);
    } else {
        struct passwd *pw = getpwuid(file->statbuf.st_uid);
        if (pw) {
            strncpy(user_str, pw->pw_name, sizeof(user_str) - 1);
        } else {
            snprintf(user_str, sizeof(user_str), "%u", file->statbuf.st_uid);
        }

        struct group *gr = getgrgid(file->statbuf.st_gid);
        if (gr) {
            strncpy(group_str, gr->gr_name, sizeof(group_str) - 1);
        } else {
            snprintf(group_str, sizeof(group_str), "%u", file->statbuf.st_gid);
        }
    }

    // Định dạng thời gian
    char time_str[64];
    get_time_string(&file->statbuf, opts, time_str, sizeof(time_str));

    // In các thông tin theo đúng thứ tự chuẩn
    printf("%s %2lu %s %s %8lld %s %s",
           mode_str,
           (unsigned long)file->statbuf.st_nlink,
           user_str,
           group_str,
           (long long)file->statbuf.st_size,
           time_str,
           file->name);

    // Nếu là Symbolic Link -> In tên file thực sự mà nó trỏ tới (-> target)
    if (S_ISLNK(file->statbuf.st_mode)) {
        char link_target[1024];
        ssize_t len = readlink(file->full_path, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf(" -> %s", link_target);
        }
    }

    // Xử lý cờ -F (phụ gia ký tự phân loại)
    if (opts->classify) {
        if (S_ISDIR(file->statbuf.st_mode)) printf("/");
        else if (S_ISLNK(file->statbuf.st_mode)) printf("@");
        else if (S_ISSOCK(file->statbuf.st_mode)) printf("=");
        else if (S_ISFIFO(file->statbuf.st_mode)) printf("|");
        else if (file->statbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) printf("*");
    }

    printf("\n");
}

void display_files(const FileList *list, const LsOptions *opts) {
    if (list->count == 0) return;

    // Nếu có cờ -l hoặc -n
    if (opts->long_format || opts->numeric_uid_gid) {
        // Tính tổng số block ổ đĩa đã sử dụng (total)
        long long total_blocks = 0;
        for (int i = 0; i < list->count; i++) {
            total_blocks += list->files[i].statbuf.st_blocks;
        }
        printf("total %lld\n", total_blocks);

        // In chi tiết từng file
        for (int i = 0; i < list->count; i++) {
            print_long_entry(&list->files[i], opts);
        }
    } else {
        // In ngắn (Mặc định: 1 tên file trên 1 dòng)
        for (int i = 0; i < list->count; i++) {
            // Xử lý cờ -i (In inode)
            if (opts->inode) {
                printf("%lu ", (unsigned long)list->files[i].statbuf.st_ino);
            }
            // Xử lý cờ -s (In số block)
            if (opts->blocks) {
                printf("%lld ", (long long)list->files[i].statbuf.st_blocks);
            }

            printf("%s", list->files[i].name);

            // Xử lý cờ -F
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