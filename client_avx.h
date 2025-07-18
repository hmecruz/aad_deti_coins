#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <omp.h>
#include "search_utilities.h"
#include "md5_cpu_avx.h"

#ifndef CLIENT_AVX
#define CLIENT_AVX

#define SERVER_IP "127.0.0.1"

// unit --> seconds

#define NUMBER_THREADS 2 // Consider the available resources of the server
#if NUMBER_THREADS < 1
    #error "NUMBER_THREADS must be 1 or greater"
#endif

#define VAR1_IDX_CLIENT_AVX 6 
#if VAR1_IDX_CLIENT_AVX < 6
    #error "VAR1_IDX_CLIENT_AVX must be 6 or greater"
#endif
#define VAR2_IDX_CLIENT_AVX 9
#if VAR2_IDX_CLIENT_AVX < 6
    #error "VAR2_IDX_CLIENT_AVX must be 6 or greater"
#endif

typedef struct {
    u32_t total_n_coins;
    u64_t total_n_attempts;
} search_result_t;

void client_search(u32_t server_port, u32_t search_time) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(server_port),
    };
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        exit(1);
    }

    // Receive mandatory prefix from server for DETI coin: client_id to be inserted into the coin
    // client_id --> 5 random characters
    char prefix[6] = {0};
    if (recv(sock_fd, prefix, sizeof(prefix) - 1, 0) <= 0) {
        perror("Failed to receive prefix");
        close(sock_fd);
        exit(1);
    }
    prefix[5] = '\0'; // Null-terminate
    printf("Received prefix: '%s'\n", prefix);

    u32_t total_n_coins = 0;      // Total number of DETI coins found
    u64_t total_n_attempts = 0;   // Total number of attempts

    time_t start_time = time(NULL);

    // Parallel region with reduction for total_n_coins and total_n_attempts
    #pragma omp parallel reduction(+:total_n_coins, total_n_attempts) num_threads(NUMBER_THREADS)
    {
        u32_t n_coins = 0;        // Coins found by this thread
        u64_t n_attempts = 0;     // Attempts made by this thread
        u32_t lane, idx;   
        coin_t coins[4]; 
        u32_t interleaved_data[13u * 4u] __attribute__((aligned(16)));
        u32_t interleaved_hash[ 4u * 4u] __attribute__((aligned(16)));

        u32_t var1 = 0x20202020;  
        u32_t var2 = 0x20202020;

        // Initialize DETI coins with the client_id, lane number and thread number
        // prefix (10) + client_id (5) + lane_number (1) + thread_number (1)
        for (lane = 0u; lane < 4u; lane++) {
            initialize_deti_coin(&coins[lane]);
            insert_text_into_coin_at(&coins[lane], prefix, 10); // Insert client_id
            coins[lane].coin_as_chars[10u + 5u] = '0' + (char)lane; // Insert the lane number
            coins[lane].coin_as_chars[10u + 5u + 1u] = '0' + (char)omp_get_thread_num(); // Insert the thread number
        }

        // Search for DETI coins
        for (n_attempts = 0ul; time(NULL) - start_time < search_time; n_attempts+=4u) {
            
            // Insert the var1 and var2 to try different combinations
            for (lane = 0u; lane < 4u; lane++) {
                coins[lane].coin_as_ints[VAR1_IDX_CLIENT_AVX] = var1;
                coins[lane].coin_as_ints[VAR2_IDX_CLIENT_AVX] = var2;
                for (idx = 0u; idx < 13u; idx++) {
                    interleaved_data[4u * idx + lane] = coins[lane].coin_as_ints[idx];
                }
            }

            // Compute MD5 hashes using AVX
            md5_cpu_avx((v4si *)interleaved_data, (v4si *)interleaved_hash);

            // Check hashes for trailing zeros
            for (lane = 0u; lane < 4u; lane++) {
                u32_t hash[4u];
                for (idx = 0u; idx < 4u; idx++) {
                    hash[idx] = interleaved_hash[4u * idx + lane];
                }

                if (hash[3] == 0x00000000){
                    // Send the coin to the server
                    if (send(sock_fd, coins[lane].coin_as_ints, sizeof(coins[lane].coin_as_ints), 0) < 0) {
                        perror("Failed to send coin");
                    }
                    n_coins++;
                    //printf("Thread %d: Found DETI coin in lane %u: %s\n",
                    //    omp_get_thread_num(), lane, coins[lane].coin_as_chars);
                }
            }

            // Update var1 and var2
            var1 = next_ascii_code(var1);
            if (var1 == 0x20202020) {
                var2 = next_ascii_code(var2);
            }
        }

        // Reduction is handled by the OpenMP reduction clause
        total_n_coins += n_coins;
        total_n_attempts += n_attempts;
    }

    // Send results to server
    search_result_t result = {
        .total_n_coins = total_n_coins,
        .total_n_attempts = total_n_attempts,
    };

    
    // Combine result and end signal into a single buffer
    char message_buffer[sizeof(search_result_t) + 1];
    memcpy(message_buffer, &result, sizeof(search_result_t));
    message_buffer[sizeof(search_result_t)] = 0; // End signal

    // Send the combined buffer
    ssize_t bytes_sent = send(sock_fd, message_buffer, sizeof(message_buffer), 0);
    if (bytes_sent < 0) {
        perror("Failed to send result and end signal");
    } else {
        printf("Sent %zd bytes\n", bytes_sent);
    }
    
    printf("Client - deti_coins_cpu_avx_openmp_search: Found %u DETI coin%s in %lu attempt%s (expected %.2f coins)\n",
        total_n_coins, (total_n_coins == 1) ? "" : "s",
        total_n_attempts, (total_n_attempts == 1) ? "" : "s",
        (double)total_n_attempts / (double)(1ul << 32));

    // Close connection
    shutdown(sock_fd, SHUT_WR);
}

#endif