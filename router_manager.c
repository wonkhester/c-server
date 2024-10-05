#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_ROUTES 10

Route routes[MAX_ROUTES];
int route_count = 0;

void add_route(const char *method, const char *path, RouteHandler handler) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].method = strdup(method);
    routes[route_count].path = strdup(path);
    routes[route_count].handler = handler;
    route_count++;
  } else {
    printf("Max number of routes reached!\n");
  }
}

RouteHandler get_route_handler(const char *method, const char *path) {
  for (int i = 0; i < route_count; i++) {

    int path_matches = (strcmp(path, routes[i].path) == 0) || (strcmp(routes[i].path, "*") == 0);
    int method_matches = (strcmp(method, routes[i].method) == 0) || (strcmp(routes[i].method, "*") == 0);

    if (path_matches && method_matches) {
      return routes[i].handler;
    }
  }
  return NULL;
}

void send_response(int client_socket, int status_code, const char *body, const char *content_type) {
  char response[1024];
  const char *status_text = (status_code == 200) ? "OK" : "Not Found";

  sprintf(response,
          "HTTP/1.1 %d %s\r\n"
          "Content-Type: %s\r\n"
          "Content-Length: %zu\r\n"
          "\r\n"
          "%s",
          status_code, status_text, content_type, strlen(body), body);

  send(client_socket, response, strlen(response), 0);
}

void handle_not_found(int client_socket, HTTP_REQUEST request) {
  char body[1024];

  sprintf(body,
          "<html><body>"
          "<h1>404 Not Found</h1>"
          "<p>The requested resource was not found on this server.</p>"
          "<h2>Request Information:</h2>"
          "<p>Method: %s</p>"
          "<p>URL: %s</p>"
          "<p>Protocol: %s</p>"
          "<p>Body: %s</p>"
          "<p>Host: %s</p>"
          "<p>User-Agent: %s</p>"
          "<p>Accept: %s</p>"
          "<p>Accept-Language: %s</p>"
          "<p>Accept-Charset: %s</p>"
          "<p>Connection: %s</p>"
          "</body></html>",
          request.method ? request.method : "(null)", request.url ? request.url : "(null)",
          request.protocol ? request.protocol : "(null)", request.body ? request.body : "(null)",
          request.headers.host ? request.headers.host : "(null)",
          request.headers.user_agent ? request.headers.user_agent : "(null)",
          request.headers.accept ? request.headers.accept : "(null)",
          request.headers.accept_language ? request.headers.accept_language : "(null)",
          request.headers.accept_charset ? request.headers.accept_charset : "(null)",
          request.headers.connection ? request.headers.connection : "(null)");

  send_response(client_socket, 404, body, "text/html");
}

HTTP_HEADERS parse_request_headers(const char *headers) {
  HTTP_HEADERS parsed_headers = {0};
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

HTTP_REQUEST parse_request(const char *request) {
  HTTP_REQUEST parsed_request = {0};
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
    parsed_request.headers = parse_request_headers(headers);
  }

  if (strcmp(parsed_request.method, "POST") == 0 || strcmp(parsed_request.method, "PUT") == 0) {
    if (body && strlen(body) > 0) {
      parsed_request.body = strdup(body);
    }
  }

  free(request_copy);
  return parsed_request;
}

void free_request(HTTP_REQUEST *request) {
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

void print_http_request(HTTP_REQUEST *request) {
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

void handle_request(int client_socket, const char *request) {
  HTTP_REQUEST parsed_request = parse_request(request);

  RouteHandler handler = get_route_handler(parsed_request.method, parsed_request.url);

  if (handler) {
    handler(client_socket, parsed_request);
  } else {
    handle_not_found(client_socket, parsed_request);
  }

  free_request(&parsed_request);
}
