#ifndef CORE_H
#define CORE_H

#include <sys/stat.h>
#include "options.h"

typedef struct {
    char name[256];
    char full_path[1024];
    struct stat statbuf;
    int stat_ok;
} FileInfo;

typedef struct {
    FileInfo *files;
    int count;
    int capacity;
} FileList;

void init_file_list(FileList *list);
void add_file_info(FileList *list, FileInfo info);
void free_file_list(FileList *list);

void process_path(const char *path, const LsOptions *opts);

#endif // CORE_H