#ifndef RESPONSE_MANAGER_H
#define RESPONSE_MANAGER_H

#include "request_manager.h"
#include <sys/socket.h>

const char *get_status_text(int status_code);

void send_http_file_response(int client_socket, HTTP_Request request, const char *file_path);
void send_http_response(int client_socket, int status_code, const char *body, const char *content_type);

#endif