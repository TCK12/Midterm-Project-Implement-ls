#ifndef SORT_H
#define SORT_H

#include "core.h"
#include "options.h"

// Hàm thực hiện sắp xếp mảng FileList dựa trên các cờ trong LsOptions
void sort_file_list(FileList *list, const LsOptions *opts);

#endif // SORT_H