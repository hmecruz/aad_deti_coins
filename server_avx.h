#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#ifndef SERVER_AVX
#define SERVER_AVX

#define MAX_CLIENTS 5
#define MAX_PENDING_CONNECTIONS 5
#define MAX_TOTAL_CONNECTIONS 10
#define PREFIX_LENGTH 5 // Do not change this value

typedef struct {
    u32_t total_n_coins;
    u64_t total_n_attempts;
} search_results_t;

// Struct for handling client threads
typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
    search_results_t client_result;
} client_data_t;

typedef struct {
    pthread_mutex_t mutex;
    int active_clients;
    int total_connections;
} server_state_t;

server_state_t server_state = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .active_clients = 0,
    .total_connections = 0,
};

// Global variables to store aggregated results and DETI coins
static u32_t total_coins = 0;
static u64_t total_attempts = 0;
static pthread_mutex_t results_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to print the server state
void print_server_state() {
    pthread_mutex_lock(&server_state.mutex);
    printf("Server State: Active Clients = %d, Total Connections = %d\n",
           server_state.active_clients, server_state.total_connections);
    pthread_mutex_unlock(&server_state.mutex);
}

// Function to generate a random prefix of a given length
void generate_random_prefix(char *prefix, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (size_t i = 0; i < length; i++) {
        prefix[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    prefix[length] = '\0'; // Null-terminate the string
}

// Function to handle communication with a single client
void *handle_client(void *arg) {
    client_data_t *client_data = (client_data_t *)arg;
    int client_fd = client_data->client_fd;
    char prefix[PREFIX_LENGTH + 1] = {0};

    // Generate and send a random prefix
    generate_random_prefix(prefix, PREFIX_LENGTH);
    if (send(client_fd, prefix, PREFIX_LENGTH, 0) < 0) {
        perror("Failed to send prefix");
        close(client_fd);
        free(client_data);
        pthread_exit(NULL);
    }
    printf("Sent prefix '%s' to client\n", prefix);

    char buffer[1024] = {0};
    int coins_received = 0;
    while (1) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break; // Client disconnected
        }

        // Process client messages
        if (bytes_received == 1 && buffer[0] == 0) { 
            // End signal
            printf("End Signal\n");
            break;
        } else if (bytes_received == sizeof(search_results_t) + 1) { 
            // Result signal + end signal
            printf("Saved the results\n");
            search_results_t *result = (search_results_t *)buffer;
            pthread_mutex_lock(&results_mutex);
            total_coins += result->total_n_coins;
            total_attempts += result->total_n_attempts;
            pthread_mutex_unlock(&results_mutex);
        } else if (bytes_received == 52) { 
            // DETI coin received
            save_deti_coin((u32_t *)buffer);
            coins_received++;
            printf("Saved Deti coin\n");
        }
        else {
            fprintf(stderr, "Received unexpected message of size %zd bytes.\n", bytes_received);
        }
    }

    printf("Client disconnected. Received %d coins.\n", coins_received);
    close(client_fd);
    free(client_data);

    pthread_mutex_lock(&server_state.mutex);
    server_state.active_clients--;
    pthread_mutex_unlock(&server_state.mutex);

    pthread_exit(NULL);
}


int server(u32_t server_port) {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;

    // Create and set up the server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("Listening failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", server_port);

    pthread_t threads[MAX_TOTAL_CONNECTIONS];
    int thread_count = 0;

    // Server loop with select() to handle non-blocking accept
    while (1) {
        pthread_mutex_lock(&server_state.mutex);

        //Server state
        //printf("Server State: Active Clients = %d, Total Connections = %d\n", 
        //    server_state.active_clients, server_state.total_connections);

        // Exit condition: No active clients and either connections occurred or limit reached
        if ((server_state.active_clients == 0 && server_state.total_connections > 0) ||
            server_state.total_connections >= MAX_TOTAL_CONNECTIONS) {
            pthread_mutex_unlock(&server_state.mutex);
            printf("All clients disconnected. Server is shutting down.\n");
            break;
        }

        pthread_mutex_unlock(&server_state.mutex);

        // Use select to avoid blocking the server
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        struct timeval timeout = {5, 0}; // Timeout of 1 second
        int activity = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("Select error");
            break;
        }

        // If there's an incoming connection, accept it
        if (FD_ISSET(server_fd, &read_fds)) {
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (client_fd < 0) {
                perror("Failed to accept connection");
                continue;
            }

            pthread_mutex_lock(&server_state.mutex);

            if (server_state.total_connections >= MAX_TOTAL_CONNECTIONS) {
                printf("Server has reached its maximum connection limit.\n");
                close(client_fd);
                pthread_mutex_unlock(&server_state.mutex);
                continue;
            }

            // Create a new thread to handle the client
            client_data_t *client_data = malloc(sizeof(client_data_t));
            client_data->client_fd = client_fd;
            client_data->client_addr = client_addr;

            if (pthread_create(&threads[thread_count], NULL, handle_client, client_data) != 0) {
                perror("Failed to create thread");
                close(client_fd);
                free(client_data);
            } else {
                server_state.active_clients++;
                server_state.total_connections++;
                thread_count++;
            }

            pthread_mutex_unlock(&server_state.mutex);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    STORE_DETI_COINS();

    // Print final aggregated results
    printf("deti_coins_cpu_avx_search: Found %u DETI coin%s in %lu attempt%s (expected %.2f coins)\n",
           total_coins, (total_coins == 1) ? "" : "s",
           total_attempts, (total_attempts == 1) ? "" : "s",
           (double)total_attempts / (double)(1ul << 32));

    close(server_fd);
    return 0;
}

#endif