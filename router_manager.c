#include "router_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h> // for close()

#define MAX_ROUTES 10

// Array to store all routes
Route routes[MAX_ROUTES];
int route_count = 0;

// Function to add a route and its handler
void add_route(const char *path, RouteHandler handler) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].path = strdup(path);
    routes[route_count].handler = handler;
    route_count++;
  } else {
    printf("Max number of routes reached!\n");
  }
}

// Function to get the handler for a given path
RouteHandler get_route_handler(const char *path) {
  for (int i = 0; i < route_count; i++) {
    if (strcmp(path, routes[i].path) == 0) {
      return routes[i].handler;
    }
  }
  return NULL; // Return NULL if no route matches
}

// Function to send an HTTP response
void send_response(int client_socket, int status_code, const char *body, const char *content_type) {
  char response[1024];
  const char *status_text = (status_code == 200) ? "OK" : "Not Found";

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
void handle_not_found(int client_socket, HTTP_REQUEST request) {
  char body[1024]; // Buffer to hold the body of the response

  // Create a detailed body that includes request information
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

  // Send the response with the formatted body
  send_response(client_socket, 404, body, "text/html");
}

// Function to parse HTTP headers
HTTP_HEADERS parse_request_headers(const char *request_headers) {
  HTTP_HEADERS parsed_request_headers = {0};

  // Duplicate the input string
  char *request_headers_copy = strdup(request_headers);
  if (!request_headers_copy) {
    fprintf(stderr, "Failed to allocate memory for headers copy.\n");
    return parsed_request_headers; // Return empty struct
  }

  char *line = strtok(request_headers_copy, "\n");
  while (line != NULL) {
    // Split header into key and value
    char *colon_pos = strchr(line, ':');
    if (colon_pos) {
      *colon_pos = '\0'; // Null-terminate key
      char *key = line;
      char *value = colon_pos + 1; // Value starts after colon

      // Trim leading whitespace from value
      while (*value == ' ')
        value++;

      // Assign values based on the key
      if (strcmp(key, "Host") == 0) {
        parsed_request_headers.host = strdup(value);
      } else if (strcmp(key, "User-Agent") == 0) {
        parsed_request_headers.user_agent = strdup(value);
      } else if (strcmp(key, "Accept") == 0) {
        parsed_request_headers.accept = strdup(value);
      } else if (strcmp(key, "Accept-Language") == 0) {
        parsed_request_headers.accept_language = strdup(value);
      } else if (strcmp(key, "Accept-Charset") == 0) {
        parsed_request_headers.accept_charset = strdup(value);
      } else if (strcmp(key, "Connection") == 0) {
        parsed_request_headers.connection = strdup(value);
      }
    }
    line = strtok(NULL, "\n"); // Move to the next line
  }

  // Clean up
  free(request_headers_copy);
  return parsed_request_headers;
}

// Function to parse the request body
char *parse_request_body(const char *request_body) {
  // Allocate memory and copy the body content
  if (request_body != NULL) {
    return strdup(request_body); // Duplicate the body string
  }
  return NULL; // Return NULL if the body is empty
}

// Function to parse the entire HTTP request
HTTP_REQUEST parse_request(const char *request) {
  HTTP_REQUEST parsed_request = {0}; // Initialize with NULLs

  // Duplicate the input string for parsing
  char *request_copy = strdup(request);
  if (!request_copy) {
    fprintf(stderr, "Failed to allocate memory for request copy.\n");
    return parsed_request; // Return empty struct
  }

  // Split the request into parts
  char *request_line = strtok(request_copy, "\n");
  char *headers = strtok(NULL, "\n");
  char *body = strtok(NULL, ""); // Get the rest of the string for the body

  // Parse method, URL, and protocol from the request line
  if (request_line) {
    parsed_request.method = strdup(strtok(request_line, " "));
    parsed_request.url = strdup(strtok(NULL, " "));
    parsed_request.protocol = strdup(strtok(NULL, " "));
  }

  // Parse the headers
  if (headers) {
    parsed_request.headers = parse_request_headers(headers);
  }

  // Parse the body
  parsed_request.body = parse_request_body(body);

  // Clean up
  free(request_copy);

  return parsed_request;
}

// Function to free dynamically allocated memory in HTTP_REQUEST
void free_request(HTTP_REQUEST *request) {
  // Free the method, URL, protocol, body, and headers
  free(request->method);
  free(request->url);
  free(request->protocol);
  free(request->body);
  free(request->headers.host);
  free(request->headers.user_agent);
  free(request->headers.accept);
  free(request->headers.accept_language);
  free(request->headers.accept_charset);
  free(request->headers.connection);
}

// Function to print the HTTP request
void print_http_request(HTTP_REQUEST *request) {
  if (request == NULL)
    return;

  printf("Method: %s\n", request->method ? request->method : "(null)");
  printf("URL: %s\n", request->url ? request->url : "(null)");
  printf("Protocol: %s\n", request->protocol ? request->protocol : "(null)");
  printf("Body: %s\n", request->body ? request->body : "(null)");
  printf("Host: %s\n", request->headers.host ? request->headers.host : "(null)");
  printf("User-Agent: %s\n", request->headers.user_agent ? request->headers.user_agent : "(null)");
  printf("Accept: %s\n", request->headers.accept ? request->headers.accept : "(null)");
  printf("Accept-Language: %s\n", request->headers.accept_language ? request->headers.accept_language : "(null)");
  printf("Accept-Charset: %s\n", request->headers.accept_charset ? request->headers.accept_charset : "(null)");
  printf("Connection: %s\n", request->headers.connection ? request->headers.connection : "(null)");
}

// Function to route the request to the appropriate handler
void handle_request(int client_socket, const char *request) {
  HTTP_REQUEST parsed_request = parse_request(request);

  print_http_request(&parsed_request); // Pass address of parsed_request

  RouteHandler handler = get_route_handler(parsed_request.url);

  if (handler) {
    handler(client_socket, parsed_request); // Call the matching handler
  } else {
    handle_not_found(client_socket, parsed_request); // No route matches, return 404
  }

  // Free the parsed request after use
  free_request(&parsed_request);
}
