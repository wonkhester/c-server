#include "router.h"
#include "file_manager.h"
#include "request_manager.h"
#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void handle_root(int client_socket, HTTP_REQUEST request) {
  char *html_content = read_html_file("index.html");
  if (html_content) {
    send_response(client_socket, 200, html_content, "text/html");
    free(html_content);
  } else {
    handle_not_found(client_socket, request);
  }
}

void handle_yo(int client_socket, HTTP_REQUEST request) {
  send_response(client_socket, 200, "This is the mega yo Page.", "text/html");
}

void handle_about(int client_socket, HTTP_REQUEST request) {
  send_response(client_socket, 200, "This is the About Page.", "text/html");
}

void handle_submit(int client_socket, HTTP_REQUEST request) {
  if (request.body != NULL) {

    ParsedData parsed_body = parse_request_body(request.body);

    char *name = get_parsed_data_key(&parsed_body, "name");
    char *age = get_parsed_data_key(&parsed_body, "age");

    char response[256];
    if (parsed_body.pairs != NULL && name != NULL && age != NULL) {
      sprintf(response, "Hello, %s! You are %s years old.", name, age);
    } else {
      sprintf(response, "Hello, unknown! You are unknown years old.");
    }

    send_response(client_socket, 200, response, "text/plain");
  } else {
    send_response(client_socket, 400, "No data received", "text/plain");
  }
}

void handle_update(int client_socket, HTTP_REQUEST request) {
  if (request.body) {
    char response_body[256];
    snprintf(response_body, sizeof(response_body), "Data has been updated with: %s", request.body);

    send_response(client_socket, 200, response_body, "text/plain");
  } else {
    send_response(client_socket, 400, "Bad Request", "text/plain");
  }
}

int init_routes() {
  add_route("GET", "/", handle_root);
  add_route("GET", "/yo", handle_yo);
  add_route("GET", "/about", handle_about);
  add_route("POST", "/submit", handle_submit);
  add_route("PUT", "/update", handle_update);
  add_route("*", "*", handle_root);
  return EXIT_SUCCESS;
}