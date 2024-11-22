//
// deti_coins_cpu_special_search() --- find DETI coins with a special form using md5_cpu()
//

#ifndef DETI_COINS_CPU_SPECIAL_SEARCH
#define DETI_COINS_CPU_SPECIAL_SEARCH

static void deti_coins_cpu_special_search(void)
{
  u32_t n, idx, coin[13u], hash[4u];
  u64_t n_attempts, n_coins;
  u08_t *bytes;
  const char *special_text = "Henrique";

  bytes = (u08_t *)&coin[0];
  //
  // mandatory for a DETI coin
  //
  bytes[0u] = 'D';
  bytes[1u] = 'E';
  bytes[2u] = 'T';
  bytes[3u] = 'I';
  bytes[4u] = ' ';
  bytes[5u] = 'c';
  bytes[6u] = 'o';
  bytes[7u] = 'i';
  bytes[8u] = 'n';
  bytes[9u] = ' ';
  //
  // arbitrary, but printable utf-8 data terminated with a '\n' is highly desirable
  //
  for (idx = 10u; idx < 13u * 4u - 1u; idx++)
    bytes[idx] = ' ';
  //
  // mandatory termination
  //
  bytes[13u * 4u - 1u] = '\n';
  //
  // find DETI coins with special text
  //
  for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts++)
  {
    //
    // Add special text to the input bytes
    //
    snprintf((char *)&bytes[10u], 13u * 4u - 11u, "%s", special_text);

    //
    // Compute MD5 hash
    //
    md5_cpu(coin, hash);
    //
    // Byte-reverse each word (that's how the MD5 message digest is printed...)
    //
    hash_byte_reverse(hash);
    //
    // Count the number of trailing zeros of the MD5 hash
    //
    n = deti_coin_power(hash);
    //
    // If the number of trailing zeros is >= 32, we have a DETI coin
    //
    if (n >= 32u)
    {
      save_deti_coin(coin);
      n_coins++;
    }
    //
    // Try next combination (byte range: 0x20..0x7E)
    //
    for (idx = 10u; idx < 13u * 4u - 1u && bytes[idx] == (u08_t)126; idx++)
      bytes[idx] = ' ';
    if (idx < 13u * 4u - 1u)
      bytes[idx]++;
  }
  STORE_DETI_COINS();
  printf("deti_coins_cpu_special_search: %lu DETI coin%s with '%s' found in %lu attempt%s (expected %.2f coins)\n",
         n_coins, (n_coins == 1ul) ? "" : "s", special_text,
         n_attempts, (n_attempts == 1ul) ? "" : "s",
         (double)n_attempts / (double)(1ul << 32));
}

#endif
