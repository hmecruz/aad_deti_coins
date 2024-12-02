//
// Arquiteturas de Alto Desempenho 2024/2025
//
// deti_coins_cuda_search() --- find DETI coins using CUDA
//

#include <stdio.h>
#include <stdlib.h>
#include "cuda_runtime.h"

#if USE_CUDA > 0
#ifndef DETI_COINS_CUDA_SEARCH
#define DETI_COINS_CUDA_SEARCH

static void deti_coins_cuda_search(u32_t n_random_words)
{
    u32_t idx, max_idx, random_word, var1, var2;
    u64_t n_attempts, n_coins;
    void *cu_params[4];

    // Variables for combination testing
    u32_t var1 = 0x20202020u; // Initial value for var1 (0x20 ASCII code for space)
    u32_t var2 = 0x20202020u; // Initial value for var2 (0x20 ASCII code for space)

    initialize_cuda(0, "deti_coins_cuda_kernel_search.cubin", "deti_coins_cuda_kernel_search", 1024u, 0u);
    max_idx = 1u; // Keep track of the maximum index in the data buffer
    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts += (64ul << 20)){
        host_data[0u] = 1u; // index zero stores the first free position in the host data array
        CU_CALL( cuMemcpyHtoD , (device_data, (void*)host_data, (size_t)1024 * sizeof(u32_t)) );
        cu_params[0] = &device_data;
        cu_params[1] = &random_word;
        cu_params[2] = &var1;
        cu_params[3] = &var2;
        CU_CALL(cuLaunchKernel, 
                (cu_kernel, 
                 (1u << 20) / 128u, // Number of blocks
                 1u, 1u,            // Grid dimensions (y, z)
                 128u, 1u, 1u,      // Block dimensions (x, y, z)
                 0u,                // Shared memory size
                 (CUstream)0,       // Stream
                 &cu_params[0],     // Kernel parameters
                 NULL));            // Extra options
        CU_CALL( cuMemcpyDtoH , ((void *)host_data, device_data, (size_t)1024 * sizeof(u32_t)) );
        if (host_data[0] > max_idx){
            max_idx = host_data[0];
        }
        for(idx = 1u; idx < host_data[0] && idx <= 1024u - 13u; idx += 13u){
            if (idx <= 1024u - 13u) {
                save_deti_coin(&host_data[idx]);
                n_coins++; // Increment the coin counter                
            }
        } 

        // Update var1 and var2 for the next iteration
        var1 = next_ascii_code(var1);
        if (var1 == 0x20202020u) {
            var2 = next_ascii_code(var2);
        }
    }  

    // Save all found DETI coins
    STORE_DETI_COINS();

    // Print results
    printf("deti_coins_cuda_search: Found %lu DETI coin%s in %lu attempt%s (expected %.2f coins)\n",
        n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s",
        (double)n_attempts / (double)(1ul << 32)); 

    terminate_cuda();
}
#endif
#endif