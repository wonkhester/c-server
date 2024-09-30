#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Winsock2.h>
#include "router.h"
#include "router_manager.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_THREADS 4  // Maximum number of worker threads
#define MAX_QUEUE 10    // Maximum number of clients waiting in the task queue

typedef struct {
    SOCKET socket;
    struct sockaddr_in client_addr;
} client_task;

// Task queue for incoming client requests
client_task task_queue[MAX_QUEUE];
int queue_front = 0, queue_rear = 0;
HANDLE queue_mutex, queue_not_empty, queue_not_full;

// Function to add a task to the queue
void enqueue_task(client_task task) {
    WaitForSingleObject(queue_not_full, INFINITE);
    WaitForSingleObject(queue_mutex, INFINITE);

    task_queue[queue_rear] = task;
    queue_rear = (queue_rear + 1) % MAX_QUEUE;

    ReleaseMutex(queue_mutex);
    ReleaseSemaphore(queue_not_empty, 1, NULL);
}

// Function to get a task from the queue
client_task dequeue_task() {
    WaitForSingleObject(queue_not_empty, INFINITE);
    WaitForSingleObject(queue_mutex, INFINITE);

    client_task task = task_queue[queue_front];
    queue_front = (queue_front + 1) % MAX_QUEUE;

    ReleaseMutex(queue_mutex);
    ReleaseSemaphore(queue_not_full, 1, NULL);

    return task;
}

// Function to handle communication with a client
DWORD WINAPI handle_client(void* arg) {
    while (1) {
        // Get a task (client) from the queue
        client_task task = dequeue_task();

        char buffer[BUFFER_SIZE] = {0};
        ssize_t valread;

        // Read data from the client (this would typically be an HTTP request)
        valread = recv(task.socket, buffer, BUFFER_SIZE, 0);
        if (valread > 0) {
            printf("Client: %s\n", buffer);
            // Pass the request to the router to handle it
            handle_request(task.socket, buffer);
            printf("Response sent to client.\n");
        } else {
            printf("Failed to read from client socket. Error: %d\n", WSAGetLastError());
        }

        // Close the socket after responding
        closesocket(task.socket);
    }
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create the server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Prepare the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the server socket to the specified port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Initialize routes
    if (init_routes() != EXIT_SUCCESS) {
      printf("Route initialization failed\n");
      closesocket(server_fd);
      WSACleanup();
      return 1;
    }

    // Create mutexes and semaphores for synchronization
    queue_mutex = CreateMutex(NULL, FALSE, NULL);
    queue_not_empty = CreateSemaphore(NULL, 0, MAX_QUEUE, NULL);
    queue_not_full = CreateSemaphore(NULL, MAX_QUEUE, MAX_QUEUE, NULL);

    // Create the worker threads (thread pool)
    HANDLE thread_pool[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_pool[i] = CreateThread(NULL, 0, handle_client, NULL, 0, NULL);
    }

    printf("Server listening on port %d\n", PORT);

    // Main server loop
    while (1) {
        // Accept a new client connection
        if ((client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            closesocket(server_fd);
            WSACleanup();
            return 1;
        }

        // Create a new client task and add it to the task queue
        client_task task;
        task.socket = client_socket;
        task.client_addr = client_addr;
        enqueue_task(task);
    }

    // Clean up and close the server socket
    closesocket(server_fd);
    WSACleanup();

    return EXIT_SUCCESS;
}
