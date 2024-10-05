#include "file_manager.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRIVATE_PATH "web/private/"

char *read_html_file(const char *filename) {
  size_t path_length = strlen(PRIVATE_PATH) + strlen(filename) + 1;
  char *filepath = (char *)malloc(path_length);
  if (!filepath) {
    fprintf(stderr, "Failed to allocate memory for file path.\n");
    return NULL;
  }

  snprintf(filepath, path_length, "%s%s", PRIVATE_PATH, filename);

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
