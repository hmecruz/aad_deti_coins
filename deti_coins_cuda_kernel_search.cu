// Arquiteturas de Alto Desempenho 2024/2025
//
// DETI Coin CUDA Kernel Code
//
// deti_coins_cuda_kernel_search() --- each thread tries 95 potential DETI coins
//

typedef unsigned int u32_t;

#include "md5.h"
#include "search_utilities.h"

//
// the nvcc compiler stores x[] and state[] in registers (constant indices!)
//
// global thread number: n = threadIdx.x + blockDim.x * blockIdx.x
// global warp number: n >> 5
// warp thread number: n & 31
//

#define VAR1_IDX_AVX 10 // between 9 and 11
#define VAR2_IDX_AVX 11 // between 9 and 11


// DETI Coin Search CUDA Kernel
extern "C" __global__ __launch_bounds__(128, 1) void deti_coins_cuda_kernel_search(u32_t *storage_area, u32_t var1, u32_t var2){
    coin_t coin;
    u32_t n, a, b, c, d, state[4], x[16], hash[4];
    n = (u32_t)threadIdx.x + (u32_t)blockDim.x * (u32_t)blockIdx.x;

    initialize_deti_coin(&coin);
    coin.coin_as_ints[VAR1_IDX_AVX] = var1;
    coin.coin_as_ints[VAR2_IDX_AVX] = var2;

    coin.coin_as_ints[4] += (n % 64) << 0; n/= 64;
    coin.coin_as_ints[4] += (n % 64) << 8; n/= 64;
    coin.coin_as_ints[4] += (n % 64) << 16; n/= 64;
    coin.coin_as_ints[4] += (n % 64) << 24; n/= 64;

    for(n = 0; n < 95; n++){
        // compute MD5 hash
        #define C(c)         (c)
        #define ROTATE(x,n)  (((x) << (n)) | ((x) >> (32 - (n))))
        #define DATA(idx)    coin.coin_as_ints[idx]
        #define HASH(idx)    hash[idx]
        #define STATE(idx)   state[idx]
        #define X(idx)       x[idx]
            CUSTOM_MD5_CODE();
            if (hash[3] == 0){
                u32_t n = atomicAdd(storage_area, 13);
                if (n + 13 <= 1024){
                    for (int j = 0; j < 13; j++) {
                        storage_area[n + j] = coin.coin_as_ints[j];
                    }
                }
            }
        coin.coin_as_ints[12] += 1 << 16; // Try different combinations
    }
        #undef C
        #undef ROTATE
        #undef DATA
        #undef HASH
        #undef STATE
        #undef X
}



    


