#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <sys/socket.h>

// Struct to represent common HTTP headers that a client might send
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

// Define the type for a route handler function
typedef void (*RouteHandler)(int client_socket, HTTP_REQUEST request);

// Define the Route structure
typedef struct {
  char *path;
  RouteHandler handler;
} Route;

// Function to add a route
void add_route(const char *path, RouteHandler handler);

// Function to get the handler for a given path
RouteHandler get_route_handler(const char *path);

// Function to send an HTTP response
void send_response(int client_socket, int status_code, const char *body, const char *content_type);

// Handler for the 404 Not Found
void handle_not_found(int client_socket, HTTP_REQUEST request);

// Function to route the request to the appropriate handler
void handle_request(int client_socket, const char *request);

#endif // ROUTE_MANAGER_H
