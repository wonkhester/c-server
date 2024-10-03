#include "router.h"
#include "file_manager.h"
#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Handler for the root ("/") route
void handle_root(int client_socket, HTTP_REQUEST request) {
  char *html_content = read_html_file("index.html");
  if (html_content) {
    send_response(client_socket, 200, html_content, "text/html");
    free(html_content); // Don't forget to free the allocated memory
  } else {
    handle_not_found(client_socket, request); // If reading fails, handle 404
  }
}

// Handler for the "/about" route
void handle_about(int client_socket, HTTP_REQUEST request) {
  send_response(client_socket, 200, "This is the About Page.", "text/html");
}

// Initialize routes
int init_routes() {
  add_route("/", handle_root);       // Register the root handler
  add_route("/about", handle_about); // Register the about handler
  return EXIT_SUCCESS;
}