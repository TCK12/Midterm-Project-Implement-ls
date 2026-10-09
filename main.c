#include <stdio.h>
#include "options.h"
#include "core.h"

int main(int argc, char *argv[]) {
    LsOptions opts;
    int first_file_index = parse_options(argc, argv, &opts);

    // Nếu không truyền đường dẫn, mặc định là thư mục hiện tại "."
    if (first_file_index == argc) {
        process_path(".", &opts);
    } else {
        for (int i = first_file_index; i < argc; i++) {
            if (argc - first_file_index > 1) {
                printf("%s:\n", argv[i]);
            }
            process_path(argv[i], &opts);
            if (i < argc - 1) {
                printf("\n");
            }
        }
    }

    return 0;
}