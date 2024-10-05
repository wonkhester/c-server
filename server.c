#include "request_manager.h"
#include "router.h"
#include "router_manager.h"
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_THREADS 4
#define MAX_QUEUE 10

typedef struct {
  int socket;
  struct sockaddr_in client_addr;
} client_task;

client_task task_queue[MAX_QUEUE];
int queue_front = 0, queue_rear = 0;
pthread_mutex_t queue_mutex;
sem_t queue_not_empty, queue_not_full;

pthread_t thread_pool[MAX_THREADS];
int server_running = 1;
int server_fd;

// Signal handler for graceful shutdown
void handle_shutdown(int sig) {
  server_running = 0;
  printf("\nShutting down server...\n");
  close(server_fd);

  // Clean up resources: threads, semaphores, mutex
  pthread_mutex_destroy(&queue_mutex);
  sem_destroy(&queue_not_empty);
  sem_destroy(&queue_not_full);

  exit(EXIT_SUCCESS);
}

void enqueue_task(client_task task) {
  sem_wait(&queue_not_full); // Block if queue is full
  pthread_mutex_lock(&queue_mutex);

  task_queue[queue_rear] = task;
  queue_rear = (queue_rear + 1) % MAX_QUEUE;

  pthread_mutex_unlock(&queue_mutex);
  sem_post(&queue_not_empty); // Signal that queue has items
}

client_task dequeue_task() {
  sem_wait(&queue_not_empty); // Block if queue is empty
  pthread_mutex_lock(&queue_mutex);

  client_task task = task_queue[queue_front];
  queue_front = (queue_front + 1) % MAX_QUEUE;

  pthread_mutex_unlock(&queue_mutex);
  sem_post(&queue_not_full); // Signal that space is available

  return task;
}

void *handle_client(void *arg) {
  while (server_running) {
    client_task task = dequeue_task();
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread;

    valread = read(task.socket, buffer, BUFFER_SIZE);
    if (valread > 0) {
      handle_http_request(task.socket, buffer);
    } else {
      printf("Failed to read from client socket. Error: %d\n", errno);
    }

    close(task.socket); // Close client socket after handling
  }
  return NULL;
}

int main() {
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  // Setup signal handling for graceful shutdown
  signal(SIGINT, handle_shutdown);

  // Create the socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    return EXIT_FAILURE;
  }

  // Prepare the server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind the socket to the specified address and port
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("Bind failed");
    close(server_fd);
    return EXIT_FAILURE;
  }

  // Set the socket to listen for incoming connections
  if (listen(server_fd, 3) == -1) {
    perror("Listen failed");
    close(server_fd);
    return EXIT_FAILURE;
  }

  // Initialize routes
  if (init_routes() != EXIT_SUCCESS) {
    printf("Route initialization failed\n");
    close(server_fd);
    return EXIT_FAILURE;
  }

  // Initialize mutex and semaphores
  pthread_mutex_init(&queue_mutex, NULL);
  sem_init(&queue_not_empty, 0, 0);
  sem_init(&queue_not_full, 0, MAX_QUEUE);

  // Create a thread pool
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_create(&thread_pool[i], NULL, handle_client, NULL);
  }

  printf("Server listening on port %d\n", PORT);

  // Main server loop
  while (server_running) {
    int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket == -1) {
      if (errno == EINTR) {
        break; // If accept is interrupted by SIGINT, exit the loop
      }
      perror("Accept failed");
      continue;
    }

    client_task task;
    task.socket = client_socket;
    task.client_addr = client_addr;
    enqueue_task(task); // Add client task to the queue
  }

  // Graceful shutdown of thread pool
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_cancel(thread_pool[i]);
    pthread_join(thread_pool[i], NULL);
  }

  return EXIT_SUCCESS;
}
