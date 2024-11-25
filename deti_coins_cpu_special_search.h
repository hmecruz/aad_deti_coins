//
// deti_coins_cpu_special_search() --- find DETI coins with a special form using md5_cpu()
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search_utilities.h" // Include the utility header

#ifndef DETI_COINS_CPU_SPECIAL_SEARCH
#define DETI_COINS_CPU_SPECIAL_SEARCH


static void deti_coins_cpu_special_search(const char *special_text)
{
    u32_t n, idx, hash[4u];
    u64_t n_attempts, n_coins;
    coin_t coin;
    
    // Validate special_text length using the utility function
    if (validate_text_length(special_text)) {
        exit(1); // Exit if the text length is invalid
    }

    // Initialize the DETI coin with the mandatory prefix and alignment
    initialize_deti_coin(&coin);

    // Insert the special text starting at the required index (10)
    insert_text_into_coin_at(&coin, special_text, 10);

    
    // Perform the search for DETI coins
    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts++) {
        // Compute MD5 hash using the coin as an array of integers
        md5_cpu(coin.coin_as_ints, hash);

        // Byte-reverse each word (for correct printing of the MD5 message digest)
        hash_byte_reverse(hash);

        // Count the number of trailing zeros of the MD5 hash
        n = deti_coin_power(hash);

        // If the number of trailing zeros is >= 32, we have a DETI coin
        if (n >= 32u) {
            save_deti_coin(coin.coin_as_ints);  // Save the coin as integers
            n_coins++;

            printf("Found DETI coin: %s\n", coin.coin_as_chars);  // Print the coin as a string
        }

        // Try the next combination (byte range: 0x20..0x7E)
        for (idx = 10u + strlen(special_text); idx < 13u * 4u - 1u && coin.coin_as_chars[idx] == (u08_t)126; idx++) {
            coin.coin_as_chars[idx] = ' ';
        }
        if (idx < 13u * 4u - 1u) {
            coin.coin_as_chars[idx]++;
        }
    }

    // Save all found DETI coins
    STORE_DETI_COINS();

    // Print the results
    printf("deti_coins_cpu_special_search: %lu DETI coin%s with '%s' found in %lu attempt%s (expected %.2f coins)\n",
           n_coins, (n_coins == 1ul) ? "" : "s", special_text,
           n_attempts, (n_attempts == 1ul) ? "" : "s",
           (double)n_attempts / (double)(1ul << 32));
}


#endif
