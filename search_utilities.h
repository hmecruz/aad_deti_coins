#ifndef SEARCH_UTILITIES_H
#define SEARCH_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the DETI coin structure
typedef union {
    u32_t coin_as_ints[13];    // 13 4-byte integers (total size 52 bytes)
    char coin_as_chars[53];    // 52 bytes = 13 * 4 bytes
} coin_t;

/**
 * @brief Maximum length of text that can fit into a DETI coin.
 *        This is calculated based on the coin size (52 bytes) minus 
 *        the prefix ("DETI coin ") and the newline character.
 */
#define MAX_SPECIAL_TEXT_LENGTH (52u - strlen("DETI coin ") - 1u)

/**
 * @brief Validates if the given text is too long to be inserted into a DETI coin.
 *
 * @param special_text The text to validate.
 * @return 0 if valid, 1 if too long.
 */
static inline int validate_text_length(const char *special_text) {
    size_t length = strlen(special_text);
    if (length > MAX_SPECIAL_TEXT_LENGTH) {
        fprintf(stderr, "Error: Text is too long (max %lu characters allowed, got %zu).\n", 
                MAX_SPECIAL_TEXT_LENGTH, length);
        return 1;
    }
    return 0;
}

/**
 * @brief Initializes a DETI coin with the mandatory prefix and alignment.
 *
 * @param coin Pointer to the coin structure to modify.
 */
static inline void initialize_deti_coin(coin_t *coin) {
    // Clear the coin's content
    memset(coin->coin_as_chars, 0, sizeof(coin->coin_as_chars));

    // Add the mandatory prefix "DETI coin "
    snprintf(coin->coin_as_chars, sizeof(coin->coin_as_chars), "DETI coin ");

    // Fill the rest with spaces and add the newline at the end
    for (size_t i = 10u; i < 51u; i++) {
        coin->coin_as_chars[i] = ' '; // Fill remaining space with spaces
    }
    coin->coin_as_chars[51] = '\n'; // Add mandatory newline at the end
    
    //coin->coin_as_chars[52] = '\0';
}

/**
 * @brief Inserts a text into a DETI coin starting at a specific index.
 *
 * @param coin Pointer to the coin structure to modify.
 * @param text The text to insert into the coin.
 * @param start_index The starting index within the coin for the text insertion.
 */
static inline void insert_text_into_coin_at(coin_t *coin, const char *text, size_t start_index) {
    size_t text_length = strlen(text);

    // Ensure the text fits within the available space
    if (start_index + text_length > 51u) {
        fprintf(stderr, "Error: Text is too long to fit into the coin at index %zu.\n", start_index);
        exit(1);
    }

    // Insert the text starting at the specified index
    memcpy(&coin->coin_as_chars[start_index], text, text_length);
}


/**
 * @brief Debug utility to print the content of a DETI coin in both string and integer form.
 *
 * @param coin Pointer to the coin structure to print.
 */
static inline void print_coin(const coin_t *coin) {
    printf("Coin as string: %.*s", (int)sizeof(coin->coin_as_chars), coin->coin_as_chars);
    printf("Coin as integers: ");
    for (size_t i = 0; i < 13u; i++) {
        printf("%08x ", coin->coin_as_ints[i]);
    }
    printf("\n");
}


/**
 * @brief Computes the next ASCII code combination in the range [0x20, 0x7E].
 *
 * @param var Current ASCII code combination (e.g., 0x20202020).
 * @return The next valid ASCII code combination or 0x20202020 if all combinations are visited.
 */
static inline u32_t next_ascii_code(u32_t var) {
    // Increment the value
    var += 1;

    // Check the first byte 
    if ((var & 0x000000FF) != 0x0000007F) {
        return var;
    }
    var += 0x000000A1;

    // Check the second byte
    if ((var & 0x0000FF00) != 0x00007F00) {
        return var;
    }
    var += 0x0000A100; 

    // Check the third byte
    if ((var & 0x00FF0000) != 0x007F0000) {
        return var;
    }
    var += 0x00A10000;

    // Check the fourth byte (most significant byte)
    if ((var & 0xFF000000) != 0x7F000000) {
        return var;
    }
    var = 0x20202020; // All combinations are visited, reset to initial state

    return var;
}


#endif // SEARCH_UTILITIES_H