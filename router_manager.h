#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <sys/socket.h>

typedef struct {
  char *host;
  char *user_agent;
  char *accept;
  char *accept_language;
  char *accept_charset;
  char *connection;
} HTTP_HEADERS;

typedef struct {
  char *method;
  char *url;
  char *protocol;
  HTTP_HEADERS headers;
  char *body;
} HTTP_REQUEST;

typedef void (*RouteHandler)(int client_socket, HTTP_REQUEST request);

typedef struct {
  char *method;
  char *path;
  RouteHandler handler;
} Route;

void add_route(const char *method, const char *path, RouteHandler handler);

RouteHandler get_route_handler(const char *method, const char *path);

void send_response(int client_socket, int status_code, const char *body, const char *content_type);

void handle_not_found(int client_socket, HTTP_REQUEST request);

void handle_request(int client_socket, const char *request);

#endif