#include "response_manager.h"
#include "request_manager.h"
#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_BUFFER_SIZE 1024

// Function to get the appropriate status text based on the status code
const char *get_status_text(int status_code) {
  switch (status_code) {
  case 200:
    return "OK";
  case 404:
    return "Not Found";
  case 500:
    return "Internal Server Error";
  case 400:
    return "Bad Request";
  case 403:
    return "Forbidden";
  case 201:
    return "Created";
  // Add other status codes as needed
  default:
    return "Unknown Status";
  }
}

// Function to send an HTTP response to the client
void send_http_response(int client_socket, int status_code, const char *body, const char *content_type) {
  // Get the status text based on the status code
  const char *status_text = get_status_text(status_code);

  // Calculate the size of the response (headers + body)
  size_t body_length = strlen(body);
  size_t response_length = snprintf(NULL, 0,
                                    "HTTP/1.1 %d %s\r\n"
                                    "Content-Type: %s\r\n"
                                    "Content-Length: %zu\r\n"
                                    "\r\n"
                                    "%s",
                                    status_code, status_text, content_type, body_length, body);

  // Allocate memory for the response
  char *response = malloc(response_length + 1);
  if (response == NULL) {
    perror("Failed to allocate memory for the response");
    return;
  }

  // Construct the HTTP response string
  sprintf(response,
          "HTTP/1.1 %d %s\r\n"
          "Content-Type: %s\r\n"
          "Content-Length: %zu\r\n"
          "\r\n"
          "%s",
          status_code, status_text, content_type, body_length, body);

  // Send the response to the client socket
  ssize_t bytes_sent = send(client_socket, response, response_length, 0);
  if (bytes_sent == -1) {
    perror("Failed to send response to the client");
  }

  // Free the allocated memory for the response
  free(response);
}
