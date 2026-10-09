#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct {
    // Nhóm ẩn/hiển thị
    int show_all;        // -a: Bao gồm file ẩn (. và ..)
    int show_almost_all; // -A: Bao gồm file ẩn nhưng trừ . và ..

    // Nhóm định dạng hiển thị
    int long_format;     // -l: In dạng dài
    int numeric_uid_gid; // -n: Giống -l nhưng dùng UID/GID thay vì name
    int classify;        // -F: Thêm ký tự đánh dấu (/ * @ % = |)
    int inode;           // -i: In số inode
    int blocks;          // -s: In số block
    int human_readable;  // -h: Hiển thị kích thước dạng dễ đọc (KB, MB...)
    int kilobyte;        // -k: Hiển thị kích thước theo KB
    int hide_non_print;  // -q: Ẩn ký tự không in được bằng dấu ?
    int raw_non_print;   // -w: In thô ký tự không in được

    // Nhóm sắp xếp
    int sort_none;       // -f: Không sắp xếp
    int sort_size;       // -S: Sắp xếp theo dung lượng
    int sort_time;       // -t: Sắp xếp theo thời gian
    int reverse;         // -r: Đảo ngược thứ tự sắp xếp
    int time_status;     // -c: Dùng ctime (status change time)
    int time_access;     // -u: Dùng atime (last access time)

    // Nhóm duyệt thư mục
    int directory_only;  // -d: Chỉ xem thông tin bản thân thư mục
    int recursive;       // -R: Duyệt đệ quy các thư mục con
} LsOptions;

// Khởi tạo tất cả cờ về 0
void init_options(LsOptions *opts);

// Hàm phân tích tham số dòng lệnh, trả về chỉ số (index) của file/thư mục đầu tiên trong argv
int parse_options(int argc, char *argv[], LsOptions *opts);

#endif // OPTIONS_H