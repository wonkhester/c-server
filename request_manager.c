#include "request_manager.h"
#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ParsedData init_parsed_data() {
  ParsedData data;
  data.pairs = NULL;
  data.count = 0;
  return data;
}

void add_pair(ParsedData *data, const char *key, const char *value) {
  data->pairs = realloc(data->pairs, (data->count + 1) * sizeof(KeyValuePair));
  if (data->pairs == NULL) {
    fprintf(stderr, "Failed to allocate memory for pairs.\n");
    exit(EXIT_FAILURE);
  }
  data->pairs[data->count].key = strdup(key);
  data->pairs[data->count].value = strdup(value);
  data->count++;
}

char *urldecode(const char *src) {
  char *decoded = malloc(strlen(src) + 1);
  char *p = decoded;

  while (*src) {
    if (*src == '%') {
      if (sscanf(src + 1, "%2x", (unsigned int *)p) == 1) {
        src += 3;
        p++;
      }
    } else if (*src == '+') {
      *p++ = ' ';
      src++;
    } else {
      *p++ = *src++;
    }
  }
  *p = '\0';
  return decoded;
}

ParsedData parse_request_body(const char *body) {
  ParsedData data = init_parsed_data();

  char *body_copy = strdup(body);
  if (!body_copy) {
    fprintf(stderr, "Failed to allocate memory for body copy.\n");
    return data;
  }

  char *pair = strtok(body_copy, "&");
  while (pair != NULL) {
    char *equals_pos = strchr(pair, '=');

    if (equals_pos != NULL) {
      *equals_pos = '\0';
      char *key = pair;
      char *value = equals_pos + 1;

      add_pair(&data, urldecode(key), urldecode(value));
    }

    pair = strtok(NULL, "&");
  }

  free(body_copy);
  return data;
}

void free_parsed_data(ParsedData *data) {
  for (size_t i = 0; i < data->count; i++) {
    free(data->pairs[i].key);
    free(data->pairs[i].value);
  }
  free(data->pairs);
}

void print_parsed_data(const ParsedData *data) {
  for (size_t i = 0; i < data->count; i++) {
    printf("%s: %s\n", data->pairs[i].key, data->pairs[i].value);
  }
}

char *get_parsed_data_key(const ParsedData *data, const char *key) {
  if (data == NULL) {
    printf("ParsedData is NULL\n");
    return NULL;
  }

  for (size_t i = 0; i < data->count; i++) {
    if (strcmp(data->pairs[i].key, key) == 0) {
      return data->pairs[i].value;
    }
  }
  return NULL;
}