//
// deti_coins_cpu_avx_search.c --- find DETI coins using AVX and md5_cpu_avx
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deti_coins_cpu_search.h"

#ifndef DETI_COINS_CPU_AVX_SEARCH
#define DETI_COINS_CPU_AVX_SEARCH

typedef int v4si __attribute__((vector_size(16)));  // AVX vector type (4 integers)

static void deti_coins_cpu_avx_search(u32_t n_random_words)
{
    u32_t n, idx, interleaved_coins[13u * 4u], interleaved_hashes[4u * 4u];
    u64_t n_attempts = 0, n_coins = 0;
    u08_t *bytes[4u];  // Pointers to each coin's bytes
    size_t lane;

    // Initialize coin data with "DETI coin " in each lane
    for (lane = 0u; lane < 4u; lane++) {
        bytes[lane] = (u08_t *)&interleaved_coins[lane];
        bytes[lane][0u] = 'D';
        bytes[lane][1u] = 'E';
        bytes[lane][2u] = 'T';
        bytes[lane][3u] = 'I';
        bytes[lane][4u] = ' ';
        bytes[lane][5u] = 'c';
        bytes[lane][6u] = 'o';
        bytes[lane][7u] = 'i';
        bytes[lane][8u] = 'n';
        bytes[lane][9u] = ' ';

        for (idx = 10u; idx < 13u * 4u - 1u; idx++) {
            bytes[lane][idx] = ' ';
        }

        bytes[lane][13u * 4u - 1u] = '\n';  // Mandatory newline
    }

    // Perform the search for DETI coins
    while (stop_request == 0) {
        n_attempts++;

        // Compute MD5 hashes for 4 interleaved coins
        md5_cpu_avx((v4si *)interleaved_coins, (v4si *)interleaved_hashes);

        for (lane = 0u; lane < 4u; lane++) {
            // Byte-reverse the hash for this lane
            hash_byte_reverse((u32_t *)&interleaved_hashes[lane]);

            // Count the number of trailing zeros of the hash
            n = deti_coin_power((u32_t *)&interleaved_hashes[lane]);

            // If the number of trailing zeros is >= 32, save the coin
            if (n >= 32u) {
                save_deti_coin(&interleaved_coins[13u * lane]);
                n_coins++;
                printf("Found DETI coin: %s\n", (char *)&interleaved_coins[13u * lane]);
                fflush(stdout);
            }
        }

        // Update the interleaved coins for the next iteration
        for (lane = 0u; lane < 4u; lane++) {
            for (idx = 10u; idx < 13u * 4u - 1u && bytes[lane][idx] == (u08_t)126; idx++) {
                bytes[lane][idx] = ' ';
            }
            if (idx < 13u * 4u - 1u) {
                bytes[lane][idx]++;
            }
        }
    }

    // Save all found DETI coins
    STORE_DETI_COINS();

    // Print results
    printf("deti_coins_cpu_avx_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n",
           n_coins, (n_coins == 1ul) ? "" : "s",
           n_attempts, (n_attempts == 1ul) ? "" : "s",
           (double)n_attempts / (double)(1ul << 32));
}

#endif
