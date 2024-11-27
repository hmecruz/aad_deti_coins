//
// deti_coins_cpu_avx_search.c --- find DETI coins using AVX and md5_cpu_avx
//

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5_cpu_avx.h"
#include "search_utilities.h"

#ifndef DETI_COINS_CPU_AVX_OPENMP_SEARCH
#define DETI_COINS_CPU_AVX_OPENMP_SEARCH

#define VAR1_IDX_AVX_THREAD 6
#if VAR1_IDX_AVX_THREAD < 5
    #error "VAR1_IDX_AVX_THREAD must be 5 or greater"
#endif
#define VAR2_IDX_AVX_THREAD 9
#if VAR2_IDX_AVX_THREAD < 5
    #error "VAR2_IDX_AVX_THREAD must be 5 or greater"
#endif

void deti_coins_cpu_avx_openmp_search(u32_t n_random_words, u32_t number_of_threads)
{
    u32_t total_n_coins = 0;      // Total number of DETI coins found
    u64_t total_n_attempts = 0;   // Total number of attempts

    // Parallel region with reduction for total_n_coins and total_n_attempts
    #pragma omp parallel reduction(+:total_n_coins, total_n_attempts) shared(stop_request) num_threads(number_of_threads)
    {
        u32_t n_coins = 0;        // Coins found by this thread
        u64_t n_attempts = 0;     // Attempts made by this thread
        u32_t n, lane, idx;   
        coin_t coins[4]; 
        u32_t interleaved_data[13u * 4u] __attribute__((aligned(16)));
        u32_t interleaved_hash[ 4u * 4u] __attribute__((aligned(16)));

        u32_t var1 = 0x20202020;  
        u32_t var2 = 0x20202020;

        // Initialize DETI coins with the lane and thread number
        for (lane = 0u; lane < 4u; lane++) {
            initialize_deti_coin(&coins[lane]);
            coins[lane].coin_as_chars[10u] = '0' + (char)lane;
            coins[lane].coin_as_chars[11u] = '0' + (char)omp_get_thread_num(); 
        }

        // Search for DETI coins
        for (n_attempts = 0ul; stop_request == 0; n_attempts+=4u) {
            
            // Insert the var1 and var2 to try different combinations
            for (lane = 0u; lane < 4u; lane++) {
                coins[lane].coin_as_ints[VAR1_IDX_AVX_THREAD] = var1;
                coins[lane].coin_as_ints[VAR2_IDX_AVX_THREAD] = var2;
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

                // Byte-reverse and check trailing zeros for each coin
                hash_byte_reverse(hash);

                // Determine DETI coin power
                n = deti_coin_power(hash);
                if (n >= 32u) {
                    save_deti_coin(coins[lane].coin_as_ints); // Save valid coin
                    n_coins++;
                    printf("Thread %d: Found DETI coin in lane %u: %s\n",
                        omp_get_thread_num(), lane, coins[lane].coin_as_chars);
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

    // Save all found DETI coins and print results
    STORE_DETI_COINS();
    printf("deti_coins_cpu_avx_openmp_search: Found %u DETI coin%s in %lu attempt%s (expected %.2f coins)\n",
        total_n_coins, (total_n_coins == 1) ? "" : "s",
        total_n_attempts, (total_n_attempts == 1) ? "" : "s",
        (double)total_n_attempts / (double)(1ul << 32));
}

#endif