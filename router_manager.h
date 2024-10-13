#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include "request_manager.h"
#include <sys/socket.h>

typedef void (*RouteHandler)(int client_socket, HTTP_Request request);

typedef struct {
  char *method;
  char *path;
  RouteHandler handler;
} Route;

void add_route(const char *method, const char *path, RouteHandler handler);

RouteHandler get_route_handler(const char *method, const char *path);

void handle_no_route(int client_socket, HTTP_Request request);

void handle_not_found(int client_socket, HTTP_Request request);

#endif