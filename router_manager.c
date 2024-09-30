#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "router_manager.h"

#define MAX_ROUTES 10

// Array to store all routes
Route routes[MAX_ROUTES];
int route_count = 0;

// Function to add a route and its handler
void add_route(const char* path, RouteHandler handler) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].path = strdup(path);
        routes[route_count].handler = handler;
        route_count++;
    } else {
        printf("Max number of routes reached!\n");
    }
}

// Function to get the handler for a given path
RouteHandler get_route_handler(const char* path) {
    for (int i = 0; i < route_count; i++) {
        if (strcmp(path, routes[i].path) == 0) {
            return routes[i].handler;
        }
    }
    return NULL;  // Return NULL if no route matches
}

// Function to send an HTTP response
void send_response(SOCKET client_socket, int status_code, const char* body, const char* content_type) {
    char response[1024];
    const char* status_text = (status_code == 200) ? "OK" : "Not Found";
    
    // Prepare the HTTP response
    sprintf(response,
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            status_code, status_text, content_type, strlen(body), body);
    
    send(client_socket, response, strlen(response), 0);
}

// Handler for the 404 Not Found
void handle_not_found(SOCKET client_socket) {
    send_response(client_socket, 404, "404 Not Found", "text/html");
}

// Function to route the request to the appropriate handler
void handle_request(SOCKET client_socket, const char* request) {
    char path[256] = {0};
    sscanf(request, "GET %s HTTP/1.1", path);

    RouteHandler handler = get_route_handler(path);

    if (handler) {
        handler(client_socket);  // Call the matching handler
    } else {
        handle_not_found(client_socket);  // No route matches, return 404
    }
}