#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "utils/string_array.h"

#define WEB_PRIVATE_PATH "web/private"
#define WEB_PUBLIC_PATH "web/private"
#define WEB_STATIC_PATH "web/public/static"
#define WEB_DYNAMIC_PATH "web/public/dynamic"
#define STATIC_FILE_INITIAL_CAPACITY 10

StringArray *get_static_web_files();

char *read_html_file(const char *filename);

long get_file_size(const char *filepath);

char *read_file(const char *filename);

StringArray *get_files_in_directory(const char *directory_path);

StringArray *get_folders_in_directory(const char *directory_path);

int init_static_web_files();

int *get_all_nested_files_in_directory(StringArray *files, const char *directory_path);

int file_exists_in_dynamic(const char *filename);

int file_exists_in_static(const char *filename);

int file_exists(const char *filename);

void free_static_web_files();

#endif
