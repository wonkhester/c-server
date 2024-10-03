#include "router.h"
#include "router_manager.h"
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_THREADS 4 // Maximum number of worker threads
#define MAX_QUEUE 10  // Maximum number of clients waiting in the task queue

typedef struct {
  int socket;
  struct sockaddr_in client_addr;
} client_task;

// Task queue for incoming client requests
client_task task_queue[MAX_QUEUE];
int queue_front = 0, queue_rear = 0;
pthread_mutex_t queue_mutex;

// Use POSIX semaphores on Linux
sem_t queue_not_empty, queue_not_full;

// Function to add a task to the queue
void enqueue_task(client_task task) {
  sem_wait(&queue_not_full);
  pthread_mutex_lock(&queue_mutex); // Lock the queue

  task_queue[queue_rear] = task;
  queue_rear = (queue_rear + 1) % MAX_QUEUE;

  pthread_mutex_unlock(&queue_mutex); // Unlock the queue

  sem_post(&queue_not_empty);
}

// Function to get a task from the queue
client_task dequeue_task() {
  sem_wait(&queue_not_empty);
  pthread_mutex_lock(&queue_mutex); // Lock the queue

  client_task task = task_queue[queue_front];
  queue_front = (queue_front + 1) % MAX_QUEUE;

  pthread_mutex_unlock(&queue_mutex); // Unlock the queue
  sem_post(&queue_not_full);

  return task;
}

// Function to handle communication with a client
void *handle_client(void *arg) {
  while (1) {
    // Get a task (client) from the queue
    client_task task = dequeue_task();
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread;

    // Read data from the client (this would typically be an HTTP request)
    valread = read(task.socket, buffer, BUFFER_SIZE);
    if (valread > 0) {
      printf("Client: %s\n", buffer);
      // Pass the request to the router_manager to handle it
      handle_request(task.socket, buffer);
      printf("Response sent to client.\n");
    } else {
      printf("Failed to read from client socket. Error: %d\n", errno);
    }

    // Close the socket after responding
    close(task.socket);
  }
  return NULL;
}

int main() {
  int server_fd, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  // Create the server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    return 1;
  }

  // Prepare the server address structure
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind the server socket to the specified port
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("Bind failed");
    close(server_fd);
    return 1;
  }

  // Start listening for incoming connections
  if (listen(server_fd, 3) == -1) {
    perror("Listen failed");
    close(server_fd);
    return 1;
  }

  // Initialize routes
  if (init_routes() != EXIT_SUCCESS) {
    printf("Route initialization failed\n");
    close(server_fd);
    return 1;
  }

  // Initialize mutex and semaphores
  pthread_mutex_init(&queue_mutex, NULL);
  sem_init(&queue_not_empty, 0, 0);
  sem_init(&queue_not_full, 0, MAX_QUEUE);

  // Create the worker threads (thread pool)
  pthread_t thread_pool[MAX_THREADS];
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_create(&thread_pool[i], NULL, handle_client, NULL);
  }

  printf("Server listening on port %d\n", PORT);

  // Main server loop
  while (1) {
    // Accept a new client connection
    if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
      perror("Accept failed");
      close(server_fd);
      return 1;
    }

    // Create a new client task and add it to the task queue
    client_task task;
    task.socket = client_socket;
    task.client_addr = client_addr;
    enqueue_task(task);
  }

  // Clean up and close the server socket
  close(server_fd);
  pthread_mutex_destroy(&queue_mutex);

  sem_destroy(&queue_not_empty);
  sem_destroy(&queue_not_full);


  return EXIT_SUCCESS;
}
