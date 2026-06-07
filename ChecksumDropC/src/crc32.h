#pragma once
#include <stdint.h>
#include <stddef.h>

/* Initializes the CRC32 lookup table if not already initialized. Thread-safe. */
void crc32_init(void);

/* Updates an existing crc value with `len` bytes from `data`. */
uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len);
