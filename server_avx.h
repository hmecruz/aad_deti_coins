#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>


#ifndef SERVER_AVX
#define SERVER_AVX

#define SERVER_PORT 12345
#define MAX_SIMULTANEOUS_CLIENTS 8
#define MAX_TOTAL_CLIENTS 100

int current_client_id = 0;
pthread_mutex_t client_id_lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    pthread_mutex_lock(&client_id_lock);
    if (current_client_id >= MAX_TOTAL_CLIENTS) {
        printf("Maximum clients reached, rejecting connection.\n");
        close(client_fd);
        pthread_mutex_unlock(&client_id_lock);
        return NULL;
    }
    int client_id = current_client_id++;
    pthread_mutex_unlock(&client_id_lock);

    // Assign mandatory prefix
    char prefix[20];
    snprintf(prefix, sizeof(prefix), "DETI coin %d", client_id);

    // Send the prefix to the client
    send(client_fd, prefix, strlen(prefix) + 1, 0);
    printf("Assigned prefix '%s' to client ID %d\n", prefix, client_id);

    // Receive results from the client (if applicable)
    char buffer[256];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Client %d: %s\n", client_id, buffer);
    }

    // Close connection
    close(client_fd);
    return NULL;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_SIMULTANEOUS_CLIENTS);

    printf("Server listening on port %d\n", SERVER_PORT);

    while (current_client_id < MAX_TOTAL_CLIENTS) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_fd < 0) {
            perror("Failed to accept client connection");
            free(client_fd);
            continue;
        }

        printf("Client connected\n");

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, client_fd);
        pthread_detach(client_thread);  // Detach thread for independent handling
    }

    close(server_fd);
    printf("Server shutting down after serving %d clients.\n", MAX_TOTAL_CLIENTS);
    return 0;
}

#endif
