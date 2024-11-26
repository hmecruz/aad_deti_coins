//
//Arquiteturas de Alto Desempenho 2024/2025
//
// MD5 hash CPU code using AVX512 instructions (Intel/AMD)
//
// md5_cpu_avx512() -------- compute the MD5 hash of a message
// test_md5_cpu_avx512() --- test the correctness of md5_cpu_avx512() and measure its execution time
//

#if defined(__GNUC__) && defined(__AVX512__)
#ifndef MD5_CPU_AVX512
#define MD5_CPU_AVX512

//
// CPU-only implementation using AVX-512 instructions (assumes a little-endian CPU)
//

typedef int v16si __attribute__ ((vector_size (64)));  // 512-bit wide registers

static void md5_cpu_avx512(v16si *interleaved4_data, v16si *interleaved4_hash)
{ 
    // Sixteen interleaved messages -> Sixteen interleaved MD5 hashes
    v16si a, b, c, d, interleaved4_state[4], interleaved4_x[16];
# define C(c)         (v16si){ (int)(c),(int)(c),(int)(c),(int)(c), (int)(c),(int)(c),(int)(c),(int)(c), \
                               (int)(c),(int)(c),(int)(c),(int)(c), (int)(c),(int)(c),(int)(c),(int)(c) }
# define ROTATE(x,n)  (_mm512_rol_epi32(x, n))
# define DATA(idx)    interleaved4_data[idx]
# define HASH(idx)    interleaved4_hash[idx]
# define STATE(idx)   interleaved4_state[idx]
# define X(idx)       interleaved4_x[idx]

    CUSTOM_MD5_CODE();

# undef C
# undef ROTATE
# undef DATA
# undef HASH
# undef STATE
# undef X
}

//
// correctness test of md5_cpu_avx512() --- test_md5_cpu_avx512() must be called first!
//

static void test_md5_cpu_avx512(void)
{
# define N_TIMING_TESTS  1000000u
    static u32_t interleaved_test_data[13u * 16u] __attribute__((aligned(64)));  // 512-bit wide, 16 interleaved
    static u32_t interleaved_test_hash[ 4u * 16u] __attribute__((aligned(64)));  // 4 hash results for each message
    u32_t n, lane, idx, *htd, *hth;

    if (N_MESSAGES % 16u != 0u)
    {
        fprintf(stderr, "test_md5_cpu_avx512: N_MESSAGES is not a multiple of 16\n");
        exit(1);
    }
    htd = &host_md5_test_data[0u];
    hth = &host_md5_test_hash[0u];
    for (n = 0u; n < N_MESSAGES; n += 16u)
    {
        //
        // interleave data
        //
        for (lane = 0u; lane < 16u; lane++)                                      // for each message number
            for (idx = 0u; idx < 13u; idx++)                                     // for each message word
                interleaved_test_data[16u * idx + lane] = htd[13u * lane + idx]; // interleave
        //
        // compute MD5 hashes
        //
        md5_cpu_avx512((v16si *)interleaved_test_data, (v16si *)interleaved_test_hash);
        //
        // compare with the test_md5_cpu() data
        //
        for (lane = 0u; lane < 16u; lane++)  // for each message number
            for (idx = 0u; idx < 4u; idx++)   // for each hash word
                if (interleaved_test_hash[16u * idx + lane] != hth[16u * lane + idx])
                {
                    fprintf(stderr, "test_md5_cpu_avx512: MD5 hash error for message %u\n", 16u * n + lane);
                    exit(1);
                }
        //
        // advance to the next 16 messages
        //
        htd = &htd[13u * 16u];
        hth = &hth[ 4u * 16u];
    }

    //
    // measure the execution time of md5_cpu_avx512()
    //
# if N_TIMING_TESTS > 0u
    time_measurement();
    for (n = 0u; n < N_TIMING_TESTS; n++)
        md5_cpu_avx512((v16si *)interleaved_test_data, (v16si *)interleaved_test_hash);
    time_measurement();
    printf("time per md5 hash (avx512): %7.3fns %7.3fns\n", cpu_time_delta_ns() / (double)(16u * N_TIMING_TESTS), wall_time_delta_ns() / (double)(16u * N_TIMING_TESTS));
# endif
# undef N_TIMING_TESTS
}

#endif
#endif
