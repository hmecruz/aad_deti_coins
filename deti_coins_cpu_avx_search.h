//
// deti_coins_cpu_avx_search.c --- find DETI coins using AVX and md5_cpu_avx
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deti_coins_cpu_search.h"
#include "md5_cpu_avx.h"

#ifndef DETI_COINS_CPU_AVX_SEARCH
#define DETI_COINS_CPU_AVX_SEARCH

static void deti_coins_cpu_avx_search(u32_t n_random_words)
{
    u32_t idx, n_coins = 0;
    u64_t n_attempts;
    coin_t coins[4];  // Process 4 coins simultaneously (x4)
    u32_t interleaved_test_data[13u * 4u] __attribute__((aligned(16)));
    u32_t interleaved_test_hash[4u * 4u] __attribute__((aligned(16)));

    // Initialize the coins
    for (int i = 0; i < 4; ++i) {
        memset(coins[i].coin_as_chars, 0, sizeof(coins[i].coin_as_chars));
        snprintf(coins[i].coin_as_chars, 52, "DETI coin %u", n_random_words + i);
    }

    // Search for DETI coins
    for (n_attempts = 0ul; stop_request == 0; n_attempts++) {
        // Interleave the coins' data for SIMD processing (4 coins at once)
        for (idx = 0u; idx < 13u; idx++) {
            for (int lane = 0; lane < 4; ++lane) {
                interleaved_test_data[4u * idx + lane] = coins[lane].coin_as_ints[idx];
            }
        }

        // Compute MD5 hashes for the interleaved coins using AVX
        md5_cpu_avx((v4si *)interleaved_test_data, (v4si *)interleaved_test_hash);

        // Check each coin's hash for trailing zeros and determine if it's a DETI coin
        for (int lane = 0; lane < 4; ++lane) {
            // Byte-reverse and check trailing zeros for each coin
            hash_byte_reverse(&interleaved_test_hash[4 * 0 + lane]);
            u32_t n = deti_coin_power(&interleaved_test_hash[4 * 0 + lane]);

            if (n >= 32u) {
                save_deti_coin(coins[lane].coin_as_ints);  // Save the valid DETI coin
                n_coins++;
                printf("Found DETI coin: %s\n", coins[lane].coin_as_chars);  // Print the found coin
            }
        }

        // Move to the next random combination for the coins (incremental search)
        for (int lane = 0; lane < 4; ++lane) {
            for (idx = 10u + n_random_words; idx < 13u * 4u - 1u && coins[lane].coin_as_chars[idx] == (u08_t)126; idx++) {
                coins[lane].coin_as_chars[idx] = ' ';
            }
            if (idx < 13u * 4u - 1u) {
                coins[lane].coin_as_chars[idx]++;
            }
        }
    }

    // Save all found DETI coins
    STORE_DETI_COINS();

    // Print results
    printf("deti_coins_cpu_avx_search: Found %u DETI coin%s in %lu attempt%s (expected %.2f coins)\n",
        n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s",
        (double)n_attempts / (double)(1ul << 32));
}

#endif
