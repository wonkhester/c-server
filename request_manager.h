#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include <string.h>

typedef struct KeyValuePair {
  char *key;
  char *value;
} KeyValuePair;

typedef struct ParsedURLData {
  KeyValuePair *pairs;
  size_t count;
} ParsedURLData;

typedef struct {
  char *host;
  char *user_agent;
  char *accept;
  char *accept_language;
  char *accept_charset;
  char *connection;
} HTTP_Headers;

typedef struct {
  char *method;
  char *url;
  char *protocol;
  HTTP_Headers headers;
  char *body;
} HTTP_Request;

ParsedURLData init_parsed_data();

void add_pair(ParsedURLData *data, const char *key, const char *value);

ParsedURLData parse_url_encoded_body(const char *body);

void free_parsed_url_data(ParsedURLData *data);

void print_parsed_data(const ParsedURLData *data);

char *get_parsed_data_key(const ParsedURLData *data, const char *key);

void handle_submit(int client_socket, HTTP_Request request);

void handle_http_request(int client_socket, const char *request);

#endif
