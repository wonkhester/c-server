#include "file_manager.h"
#include "request_manager.h"
#include "utils/string_array.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

StringArray *static_web_files;

StringArray *get_static_web_files() { return static_web_files; }

char *read_html_file(const char *filename) {
  size_t path_length = strlen(WEB_PRIVATE_PATH) + strlen(filename) + 2;
  char *filepath = (char *)malloc(path_length);
  if (!filepath) {
    fprintf(stderr, "Failed to allocate memory for file path.\n");
    return NULL;
  }

  snprintf(filepath, path_length, "%s/%s", WEB_PRIVATE_PATH, filename);

  FILE *file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "Failed to open file '%s': %s\n", filepath, strerror(errno));
    free(filepath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *content = (char *)malloc(file_size + 1);
  if (!content) {
    fprintf(stderr, "Failed to allocate memory for file content.\n");
    fclose(file);
    free(filepath);
    return NULL;
  }

  fread(content, 1, file_size, file);
  content[file_size] = '\0';

  fclose(file);
  free(filepath);
  return content;
}

long get_file_size(const char *filepath) {
  struct stat file_stat;

  if (stat(filepath, &file_stat) != 0) {
    perror("Failed to get file status");
    return -1;
  }

  return file_stat.st_size;
}

int is_text_file(const char *filepath) {
  const char *ext = strrchr(filepath, '.');
  if (ext) {
    return (strcmp(ext, ".txt") == 0);
  }
  return 0;
}

char *read_file(const char *filepath) {
  int is_binary = !is_text_file(filepath);
  long file_size = get_file_size(filepath);

  if (file_size == -1) {
    return NULL;
  }

  FILE *file = fopen(filepath, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file '%s': %s\n", filepath, strerror(errno));
    return NULL;
  }

  char *content = (char *)malloc(file_size + (is_binary ? 0 : 1));
  if (!content) {
    fprintf(stderr, "Failed to allocate memory for file content.\n");
    fclose(file);
    return NULL;
  }

  size_t bytes_read = fread(content, 1, file_size, file);
  if (bytes_read != file_size) {
    fprintf(stderr, "Failed to read the entire file. Expected: %ld, Got: %zu\n", file_size, bytes_read);
    free(content);
    fclose(file);
    return NULL;
  }

  if (!is_binary) {
    content[file_size] = '\0';
  }

  fclose(file);
  return content;
}

StringArray *get_files_in_directory(const char *directory_path) {
  StringArray *file_names = (StringArray *)malloc(sizeof(StringArray));
  if (!file_names) {
    perror("Failed to allocate memory for file names array");
    return NULL;
  }

  init_string_array(file_names, STATIC_FILE_INITIAL_CAPACITY);

  struct dirent *dp;
  struct stat path_stat;
  DIR *dir = opendir(directory_path);

  if (!dir) {
    perror("Unable to read directory");
    free(file_names);
    return NULL;
  }

  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", directory_path, dp->d_name);

    if (stat(path, &path_stat) != 0) {
      perror("Failed to get file status");
      continue;
    }

    if (S_ISDIR(path_stat.st_mode)) {
      continue;
    }

    add_to_string_array(file_names, path);
  }

  closedir(dir);
  return file_names;
}

StringArray *get_folders_in_directory(const char *directory_path) {
  StringArray *directory_names = (StringArray *)malloc(sizeof(StringArray));
  if (!directory_names) {
    perror("Failed to allocate memory for directory names array");
    return NULL;
  }

  init_string_array(directory_names, STATIC_FILE_INITIAL_CAPACITY);

  struct dirent *dp;
  struct stat path_stat;
  DIR *dir = opendir(directory_path);

  if (!dir) {
    perror("Unable to read directory");
    free(directory_names);
    return NULL;
  }

  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", directory_path, dp->d_name);

    if (stat(path, &path_stat) != 0) {
      perror("Failed to get file status");
      continue;
    }

    if (!S_ISDIR(path_stat.st_mode)) {
      continue;
    }

    add_to_string_array(directory_names, dp->d_name);
  }

  closedir(dir);
  return directory_names;
}

int init_static_web_files() {
  if (static_web_files == NULL) {
    static_web_files = (StringArray *)malloc(sizeof(StringArray));
    if (!static_web_files) {
      perror("Failed to allocate memory for static web files");
      return EXIT_FAILURE;
    }
    init_string_array(static_web_files, STATIC_FILE_INITIAL_CAPACITY);
  }

  if (get_all_nested_files_in_directory(static_web_files, WEB_STATIC_PATH) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  print_string_array(get_static_web_files());

  return EXIT_SUCCESS;
}

int *get_all_nested_files_in_directory(StringArray *files, const char *directory_path) {
  StringArray *new_files = get_files_in_directory(directory_path);
  if (new_files) {
    merge_modify_string_arrays(files, new_files);
    free_string_array(new_files);
    free(new_files);
  }

  StringArray *directories = get_folders_in_directory(directory_path);
  if (directories) {
    for (int i = 0; i < directories->count; i++) {
      char subdirectory_path[1024];
      snprintf(subdirectory_path, sizeof(subdirectory_path), "%s/%s", directory_path, directories->array[i]);

      get_all_nested_files_in_directory(files, subdirectory_path);
    }
    free_string_array(directories);
  }
  return EXIT_SUCCESS;
}

int file_exists_in_dynamic(const char *filepath) {
  struct stat buffer;
  return (stat(filepath, &buffer) == 0);
}

int file_exists_in_static(const char *filename) {
  for (int i = 0; i < static_web_files->count; i++) {
    if (strcmp(static_web_files->array[i], filename) == 0) {
      return 1;
    }
  }
  return 0;
}

int file_exists(const char *filename) {
  if (file_exists_in_static(filename)) {
    return 1;
  }

  return file_exists_in_dynamic(filename);
}

void free_static_web_files() {
  free_string_array(static_web_files);
  free(static_web_files);
}