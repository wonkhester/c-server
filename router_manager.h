#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <Winsock2.h>

// Define the type for a route handler function
typedef void (*RouteHandler)(SOCKET client_socket);

// Define the Route structure
typedef struct {
    char* path;
    RouteHandler handler;
} Route;

// Function to add a route
void add_route(const char* path, RouteHandler handler);

// Function to get the handler for a given path
RouteHandler get_route_handler(const char* path);

// Function to send an HTTP response
void send_response(SOCKET client_socket, int status_code, const char* body, const char* content_type);

// Handler for the 404 Not Found
void handle_not_found(SOCKET client_socket);

// Function to route the request to the appropriate handler
void handle_request(SOCKET client_socket, const char* request);

#endif // ROUTE_MANAGER_H
