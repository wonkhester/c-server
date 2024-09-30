#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // For error reporting
#include "router_manager.h"

#define PRIVATE_PATH "web/private/"

// Function to read an HTML file and return its contents as a string
char* read_html_file(const char* filename) {
    // Allocate memory for the file path
    // +1 for the null terminator, +1 for the '/' character if needed
    size_t path_length = strlen(PRIVATE_PATH) + strlen(filename) + 1; // +1 for null terminator
    char* filepath = (char*)malloc(path_length);
    if (!filepath) {
        fprintf(stderr, "Failed to allocate memory for file path.\n");
        return NULL;
    }

    // Construct the file path
    snprintf(filepath, path_length, "%s%s", PRIVATE_PATH, filename);

    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file '%s': %s\n", filepath, strerror(errno));
        free(filepath); // Free the filepath memory before returning
        return NULL;
    }

    // Move to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);  // Move back to the beginning of the file

    // Allocate memory for the file content
    char* content = (char*)malloc(file_size + 1);  // +1 for null terminator
    if (!content) {
        fprintf(stderr, "Failed to allocate memory for file content.\n");
        fclose(file);
        free(filepath); // Free the filepath memory before returning
        return NULL;
    }

    // Read the file into memory
    fread(content, 1, file_size, file);
    content[file_size] = '\0';  // Null-terminate the string

    fclose(file);
    free(filepath); // Free the filepath memory after use
    return content;
}
