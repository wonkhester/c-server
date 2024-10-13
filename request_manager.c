#include "request_manager.h"
#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ParsedURLData init_parsed_data() {
  ParsedURLData data;
  data.pairs = NULL;
  data.count = 0;
  return data;
}

void add_pair(ParsedURLData *data, const char *key, const char *value) {
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

ParsedURLData parse_url_encoded_body(const char *body) {
  ParsedURLData data = init_parsed_data();

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

void free_parsed_url_data(ParsedURLData *data) {
  for (size_t i = 0; i < data->count; i++) {
    free(data->pairs[i].key);
    free(data->pairs[i].value);
  }
  free(data->pairs);
}

void print_parsed_data(const ParsedURLData *data) {
  for (size_t i = 0; i < data->count; i++) {
    printf("%s: %s\n", data->pairs[i].key, data->pairs[i].value);
  }
}

char *get_parsed_data_key(const ParsedURLData *data, const char *key) {
  if (data == NULL) {
    printf("ParsedURLData is NULL\n");
    return NULL;
  }

  for (size_t i = 0; i < data->count; i++) {
    if (strcmp(data->pairs[i].key, key) == 0) {
      return data->pairs[i].value;
    }
  }
  return NULL;
}

HTTP_Headers parse_http_request_headers(const char *headers) {
  HTTP_Headers parsed_headers = {0};
  char *headers_copy = strdup(headers);
  char *line = strtok(headers_copy, "\r\n");

  while (line) {
    char *colon_pos = strchr(line, ':');
    if (colon_pos) {
      *colon_pos = '\0';
      char *key = line;
      char *value = colon_pos + 1;

      while (*value == ' ')
        value++;

      if (strcmp(key, "Host") == 0) {
        parsed_headers.host = strdup(value);
      } else if (strcmp(key, "User-Agent") == 0) {
        parsed_headers.user_agent = strdup(value);
      } else if (strcmp(key, "Accept") == 0) {
        parsed_headers.accept = strdup(value);
      } else if (strcmp(key, "Accept-Language") == 0) {
        parsed_headers.accept_language = strdup(value);
      } else if (strcmp(key, "Accept-Charset") == 0) {
        parsed_headers.accept_charset = strdup(value);
      } else if (strcmp(key, "Connection") == 0) {
        parsed_headers.connection = strdup(value);
      }
    }
    line = strtok(NULL, "\r\n");
  }

  free(headers_copy);
  return parsed_headers;
}

HTTP_Request parse_http_request(const char *request) {
  HTTP_Request parsed_request = {0};
  char *request_copy = strdup(request);

  char *headers_end = strstr(request_copy, "\r\n\r\n");
  if (!headers_end) {
    fprintf(stderr, "Invalid request, no header-body separation found\n");
    free(request_copy);
    return parsed_request;
  }

  *headers_end = '\0';
  char *body = headers_end + 4;

  char *request_line = strtok(request_copy, "\r\n");
  char *headers = strtok(NULL, "");

  if (request_line) {
    parsed_request.method = strdup(strtok(request_line, " "));
    parsed_request.url = strdup(strtok(NULL, " "));
    parsed_request.protocol = strdup(strtok(NULL, " "));
  }

  if (headers) {
    parsed_request.headers = parse_http_request_headers(headers);
  }

  if (strcmp(parsed_request.method, "POST") == 0 || strcmp(parsed_request.method, "PUT") == 0) {
    if (body && strlen(body) > 0) {
      parsed_request.body = strdup(body);
    }
  }

  free(request_copy);
  return parsed_request;
}

void free_request(HTTP_Request *request) {
  free(request->method);
  free(request->url);
  free(request->protocol);
  free(request->body);
  free(request->headers.host);
  free(request->headers.user_agent);
  free(request->headers.accept);
  free(request->headers.accept_language);
  free(request->headers.accept_charset);
  free(request->headers.connection);
}

void print_HTTP_Request(HTTP_Request *request) {
  if (request == NULL)
    return;

  printf("Method: %s\n", request->method ? request->method : "(null)");
  printf("URL: %s\n", request->url ? request->url : "(null)");
  printf("Protocol: %s\n", request->protocol ? request->protocol : "(null)");
  printf("Body: %s\n", request->body ? request->body : "(null)");
  printf("Host: %s\n", request->headers.host ? request->headers.host : "(null)");
  printf("User-Agent: %s\n", request->headers.user_agent ? request->headers.user_agent : "(null)");
  printf("Accept: %s\n", request->headers.accept ? request->headers.accept : "(null)");
  printf("Accept-Language: %s\n", request->headers.accept_language ? request->headers.accept_language : "(null)");
  printf("Accept-Charset: %s\n", request->headers.accept_charset ? request->headers.accept_charset : "(null)");
  printf("Connection: %s\n", request->headers.connection ? request->headers.connection : "(null)");
}

void handle_http_request(int client_socket, const char *request) {
  HTTP_Request parsed_request = parse_http_request(request);

  RouteHandler handler = get_route_handler(parsed_request.method, parsed_request.url);
  if (handler) {
    handler(client_socket, parsed_request);
  } else {
    handle_no_route(client_socket, parsed_request);
  }

  free_request(&parsed_request);
}
