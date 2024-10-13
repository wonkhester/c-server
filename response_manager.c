#include "response_manager.h"
#include "file_manager.h"
#include "request_manager.h"
#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

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
  default:
    return "Unknown Status";
  }
}

const char *get_content_type(const char *file_path) {
  if (strstr(file_path, ".html"))
    return "text/html";
  if (strstr(file_path, ".css"))
    return "text/css";
  if (strstr(file_path, ".js"))
    return "application/javascript";
  if (strstr(file_path, ".png"))
    return "image/png";
  if (strstr(file_path, ".jpg") || strstr(file_path, ".jpeg"))
    return "image/jpeg";
  return "application/octet-stream";
}

void send_http_file_response(int client_socket, HTTP_Request request, const char *file_path) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    handle_not_found(client_socket, request);
    return;
  }

  const char *content_type = get_content_type(file_path);

  char header[256];
  snprintf(header, sizeof(header),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %ld\r\n"
           "\r\n",
           content_type, get_file_size(file_path));

  send(client_socket, header, strlen(header), 0);

  char buffer[BUFFER_SIZE];
  size_t bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    send(client_socket, buffer, bytes_read, 0);
  }

  fclose(file);
}

void send_http_response(int client_socket, int status_code, const char *body, const char *content_type) {
  const char *status_text = get_status_text(status_code);

  size_t body_length = strlen(body);
  size_t response_length = snprintf(NULL, 0,
                                    "HTTP/1.1 %d %s\r\n"
                                    "Content-Type: %s\r\n"
                                    "Content-Length: %zu\r\n"
                                    "\r\n"
                                    "%s",
                                    status_code, status_text, content_type, body_length, body);

  char *response = malloc(response_length + 1);
  if (response == NULL) {
    perror("Failed to allocate memory for the response");
    return;
  }

  sprintf(response,
          "HTTP/1.1 %d %s\r\n"
          "Content-Type: %s\r\n"
          "Content-Length: %zu\r\n"
          "\r\n"
          "%s",
          status_code, status_text, content_type, body_length, body);

  ssize_t bytes_sent = send(client_socket, response, response_length, 0);
  if (bytes_sent == -1) {
    perror("Failed to send response to the client");
  }

  free(response);
}
