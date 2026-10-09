#include <stdio.h>
#include "display.h"

void display_files(const FileList *list, const LsOptions *opts) {
    (void)opts; // Tạm thời chưa dùng opts ở bước hiển thị cơ bản
    for (int i = 0; i < list->count; i++) {
        printf("%s\n", list->files[i].name);
    }
}