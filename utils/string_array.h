#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char **array;
  size_t count;
  size_t capacity;
} StringArray;

void init_string_array(StringArray *arr, size_t initial_capacity);

void add_to_string_array(StringArray *arr, const char *str);

void merge_modify_string_arrays(StringArray *dest, const StringArray *src);

StringArray merge_to_new_string_arrays(const StringArray *arr1, const StringArray *arr2);

void print_string_array(StringArray *arr);

void free_string_array(StringArray *arr);

#endif
