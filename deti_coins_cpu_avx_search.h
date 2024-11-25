//
// deti_coins_cpu_avx_search.c --- find DETI coins using AVX and md5_cpu_avx
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_utilities.h"
#include "md5_cpu_avx.h"

#ifndef DETI_COINS_CPU_AVX_SEARCH
#define DETI_COINS_CPU_AVX_SEARCH

#define VAR1_IDX 5
#define VAR2_IDX 6

static void deti_coins_cpu_avx_search(u32_t n_random_words)
{
    u32_t n, lane, idx, n_coins = 0;
    u64_t n_attempts;
    coin_t coins[4]; 
    u32_t interleaved_test_data[13u * 4u] __attribute__((aligned(16)));
    u32_t interleaved_test_hash[4u * 4u] __attribute__((aligned(16)));

    // Variables for combination testing
    u32_t var1 = 0x20202020; // Initial value for var1 (0x20 ASCII code for space)
    u32_t var2 = 0x20202020; // Initial value for var2 (0x20 ASCII code for space)

    // Initialize the DETI coins with the mandatory prefix and alignment
    for (lane = 0u; lane < 4u; lane++) {
        initialize_deti_coin(&coins[lane]);
        coins[lane].coin_as_chars[10u] = '0' + (char)lane; 

        printf("Initialized DETI coin %u: %s\n", lane, coins[lane].coin_as_chars);
    }

    // Interleave the coin data into the interleaved_test_data array
    for (idx = 0u; idx < 13u; idx++) { // Loop through each word of a coin
        for (lane = 0u; lane < 4u; lane++) { // Loop through each coin
            interleaved_test_data[4u * idx + lane] = coins[lane].coin_as_ints[idx];
        }
    }
    
    // Print the interleaved_test_data 
    //printf("Interleaved Test Data:\n");
    //for (idx = 0u; idx < 13u; idx++) {
    //    printf("Word %2u: ", idx);
    //    for (lane = 0u; lane < 4u; lane++) {
    //        printf("Lane %u: 0x%08x ", lane, interleaved_test_data[4u * idx + lane]);
    //    }
    //    printf("\n");
    //}

    // Search for DETI coins
    for (n_attempts = 0ul; stop_request == 0; n_attempts+=4) {

        // Insert the var1 and var2 to try different combinations
        for (lane = 0u; lane < 4u; lane++) {
            coins[lane].coin_as_ints[VAR1_IDX] = var1;
            coins[lane].coin_as_ints[VAR2_IDX] = var2;
            for (idx = 0u; idx < 13u; idx++) {
                interleaved_test_data[4u * idx + lane] = coins[lane].coin_as_ints[idx];
            }
        }

        // Print the interleaved_test_data 
        printf("Interleaved Test Data:\n");
        for (idx = 0u; idx < 13u; idx++) {
            printf("Word %2u: ", idx);
            for (lane = 0u; lane < 4u; lane++) {
                printf("Lane %u: 0x%08x ", lane, interleaved_test_data[4u * idx + lane]);
            }
            printf("\n");
        }

        //sleep(1);
        
        // Compute MD5 hashes for the interleaved coins using AVX
        md5_cpu_avx((v4si *)interleaved_test_data, (v4si *)interleaved_test_hash);

        // Check each coin's hash for trailing zeros and determine if it's a DETI coin
        for (lane = 0u; lane < 4u; lane++) {
            u32_t hash[4u];
            for (idx = 0u; idx < 4u; idx++) {
                hash[idx] = interleaved_test_hash[4u * idx + lane];
            }
            // Byte-reverse and check trailing zeros for each coin
            hash_byte_reverse(hash);
            n = deti_coin_power(hash);

            if (n >= 32u) {
                save_deti_coin(coins[lane].coin_as_ints);  // Save the valid DETI coin
                n_coins++;
                printf("Found DETI coin: %s\n", coins[lane].coin_as_chars);  // Print the found coin
            }
        }

        // Update var1 and var2
        var1 = next_ascii_code(var1);
        if (var1 == 0x20202020) {  // If var1 overflows, increment var2
            var2 = next_ascii_code(var2);
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
