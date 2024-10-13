#include "string_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_string_array(StringArray *arr, size_t initial_capacity) {
  arr->count = 0;
  arr->capacity = initial_capacity;
  arr->array = (char **)malloc(arr->capacity * sizeof(char *));
  if (!arr->array) {
    perror("Failed to allocate memory");
    exit(EXIT_FAILURE);
  }
}

void add_to_string_array(StringArray *arr, const char *str) {
  if (arr->count >= arr->capacity) {
    arr->capacity *= 2;
    arr->array = (char **)realloc(arr->array, arr->capacity * sizeof(char *));
    if (!arr->array) {
      perror("Failed to reallocate memory");
      exit(EXIT_FAILURE);
    }
  }

  arr->array[arr->count] = strdup(str);
  if (!arr->array[arr->count]) {
    perror("Failed to allocate memory for string");
    exit(EXIT_FAILURE);
  }
  arr->count++;
}

void merge_modify_string_arrays(StringArray *dest, const StringArray *src) {
  for (size_t i = 0; i < src->count; i++) {
    add_to_string_array(dest, src->array[i]);
  }
}

StringArray merge_to_new_string_arrays(const StringArray *arr1, const StringArray *arr2) {
  StringArray result;
  init_string_array(&result, arr1->count + arr2->count);

  for (size_t i = 0; i < arr1->count; i++) {
    add_to_string_array(&result, arr1->array[i]);
  }

  for (size_t i = 0; i < arr2->count; i++) {
    add_to_string_array(&result, arr2->array[i]);
  }

  return result;
}

void print_string_array(StringArray *arr) {
  for (size_t i = 0; i < arr->count; i++) {
    printf("%ld: %s\n", i, arr->array[i]);
  }
}

void free_string_array(StringArray *arr) {
  for (size_t i = 0; i < arr->count; i++) {
    free(arr->array[i]);
  }
  free(arr->array);
}
