#pragma once
#include "checksum_method.h"

/* Computes the specified `method` checksum over the file at `path`,
 * and writes the upper-case hexadecimal hash string into `out_hex`.
 *
 * `out_hex` must be pre-allocated and big enough to hold the maximum hash
 * representation plus the null terminator (at least 129 bytes: 128 chars + NUL).
 *
 * Returns 0 on success, or:
 *   -1 if the file cannot be opened / read
 *   -2 if the method is invalid / out of range
 */
int compute_hash(const char *path, ChecksumMethod method, char *out_hex);
