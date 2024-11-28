#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
//#include "deti_coins_vault.h"

#ifndef SERVER_AVX
#define SERVER_AVX

#define MAX_CLIENTS 10
#define PREFIX_LENGTH 5 // Do not change this value
#define MAX_DETI_COINS 500 

typedef struct {
    u32_t total_n_coins;
    u64_t total_n_attempts;
} search_results_t;

// Global variables to store aggregated results and DETI coins
static u32_t total_coins = 0;
static u64_t total_attempts = 0;
static u32_t deti_coin_count = 0;

// Function to generate a random prefix of a given length
void generate_random_prefix(char *prefix, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (size_t i = 0; i < length; i++) {
        prefix[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    prefix[length] = '\0'; // Null-terminate the string
}

void handle_client(int client_sock) {
    char prefix[PREFIX_LENGTH + 1]; // +1 for null terminator
    generate_random_prefix(prefix, PREFIX_LENGTH);

    // Send the prefix to the client
    if (send(client_sock, prefix, PREFIX_LENGTH + 1, 0) < 0) {
        perror("Failed to send prefix to client");
        close(client_sock);
        return;
    }
    printf("Sent prefix '%s' to client\n", prefix);

    // Receive coins and results from the client
    search_results_t result = {0};
    while (1) {
        u32_t coin_buffer[13]; 

        ssize_t bytes_received = recv(client_sock, coin_buffer, sizeof(coin_buffer), 0);
        if (bytes_received <= 0) {
            // No more coins or client disconnected
            break;
        }

        // Log the received coin
        save_deti_coin(coin_buffer);  // Save the valid DETI coin
    }

    // Receive final result summary from the client
    ssize_t bytes_received = recv(client_sock, &result, sizeof(result), 0);
    if (bytes_received > 0) {
        printf("Client summary:\n");
        printf(" - Total coins found: %u\n", result.total_n_coins);
        printf(" - Total attempts: %lu\n", result.total_n_attempts);

        // Aggregate results
        total_coins += result.total_n_coins;
        total_attempts += result.total_n_attempts;
    } else {
        perror("Failed to receive final result from client");
    }


    // Close the client socket
    close(client_sock);
}

void server(u32_t server_port) {
    srand(time(NULL)); // Seed the random number generator

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(server_port),
        .sin_addr.s_addr = INADDR_ANY,
    };

    // Bind the server socket to the specified port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Failed to listen on socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", server_port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Failed to accept client connection");
            continue;
        }

        printf("Accepted connection from client\n");

        // Handle the client in a separate function
        handle_client(client_sock);

        // Check if all clients are processed (example mechanism)
        if (deti_coin_count >= MAX_DETI_COINS) {
            printf("Maximum coin storage reached. Stopping server.\n");
            break;
    }

    STORE_DETI_COINS();

    printf("deti_coins_cpu_avx_search: Found %u DETI coin%s in %lu attempt%s (expected %.2f coins)\n",
        total_coins, (total_coins == 1) ? "" : "s", total_attempts,
        (total_attempts == 1) ? "" : "s",
        (double)total_attempts / (double)(1ul << 32));

    // Close the server socket (never reached in this example)
    close(server_sock);
    }
}

#endif