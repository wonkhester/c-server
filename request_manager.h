#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include "router_manager.h"
#include <string.h>

typedef struct KeyValuePair {
  char *key;
  char *value;
} KeyValuePair;

typedef struct ParsedData {
  KeyValuePair *pairs;
  size_t count;
} ParsedData;

ParsedData init_parsed_data();

void add_pair(ParsedData *data, const char *key, const char *value);

ParsedData parse_request_body(const char *body);

void free_parsed_data(ParsedData *data);

void print_parsed_data(const ParsedData *data);

char *get_parsed_data_key(const ParsedData *data, const char *key);

void handle_submit(int client_socket, HTTP_REQUEST request);

#endif
