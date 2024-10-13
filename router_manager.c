#include "router_manager.h"
#include "file_manager.h"
#include "request_manager.h"
#include "response_manager.h"
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

void handle_no_route(int client_socket, HTTP_Request request) {
  char static_file_path[1024];
  char dynamic_file_path[1024];

  if (request.url[0] == '/') {
    snprintf(static_file_path, sizeof(static_file_path), "%s%s", WEB_STATIC_PATH, request.url);
    snprintf(dynamic_file_path, sizeof(dynamic_file_path), "%s%s", WEB_DYNAMIC_PATH, request.url);
  } else {
    snprintf(static_file_path, sizeof(static_file_path), "%s/%s", WEB_STATIC_PATH, request.url);
    snprintf(dynamic_file_path, sizeof(dynamic_file_path), "%s/%s", WEB_DYNAMIC_PATH, request.url);
  }

  if (file_exists_in_static(static_file_path)) {
    send_http_file_response(client_socket, request, static_file_path);
  } else if (file_exists_in_dynamic(dynamic_file_path)) {
    send_http_file_response(client_socket, request, dynamic_file_path);
  } else {
    handle_not_found(client_socket, request);
  }
}

void handle_not_found(int client_socket, HTTP_Request request) {
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

  send_http_response(client_socket, 404, body, "text/html");
}