#define XXH_INLINE_ALL
#include "hashing.h"
#include "crc32.h"
#include "md5.h"
#include "sha1.h"
#include "sha2.h"
#include "sha3.h"
#include "blake3.h"
#include "xxhash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper to convert a byte array to uppercase hex string */
static void to_hex_upper(const uint8_t *bytes, size_t n, char *out_hex) {
    static const char hex_chars[] = "0123456789ABCDEF";
    for (size_t i = 0; i < n; i++) {
        out_hex[i * 2]     = hex_chars[(bytes[i] >> 4) & 0x0F];
        out_hex[i * 2 + 1] = hex_chars[bytes[i] & 0x0F];
    }
    out_hex[n * 2] = '\0';
}

/* Helper to convert a 64-bit integer to big-endian hex string (for xxHash) */
static void uint64_to_hex_be(uint64_t val, char *out_hex) {
    static const char hex_chars[] = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        uint8_t b = (uint8_t)((val >> (8 * (7 - i))) & 0xFF);
        out_hex[i * 2]     = hex_chars[(b >> 4) & 0x0F];
        out_hex[i * 2 + 1] = hex_chars[b & 0x0F];
    }
    out_hex[16] = '\0';
}

static int hash_crc32(FILE *f, char *out_hex) {
    uint32_t crc = 0xFFFFFFFF;
    uint8_t buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        crc = crc32_update(crc, buffer, bytes_read);
    }
    crc = crc ^ 0xFFFFFFFF;
    sprintf(out_hex, "%08X", crc);
    return 0;
}

static int hash_md5(FILE *f, char *out_hex) {
    MD5_CTX ctx;
    md5_init(&ctx);
    uint8_t buffer[16384];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        md5_update(&ctx, buffer, bytes_read);
    }
    uint8_t digest[16];
    md5_final(&ctx, digest);
    to_hex_upper(digest, 16, out_hex);
    return 0;
}

static int hash_sha1(FILE *f, char *out_hex) {
    SHA1_CTX ctx;
    sha1_init(&ctx);
    uint8_t buffer[16384];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha1_update(&ctx, buffer, bytes_read);
    }
    uint8_t digest[20];
    sha1_final(&ctx, digest);
    to_hex_upper(digest, 20, out_hex);
    return 0;
}

static int hash_sha2(FILE *f, ChecksumMethod method, char *out_hex) {
    uint8_t buffer[16384];
    size_t bytes_read;
    if (method == METHOD_SHA256) {
        sha256_ctx ctx;
        sha256_init(&ctx);
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            sha256_update(&ctx, (const uint8 *)buffer, bytes_read);
        }
        uint8_t digest[32];
        sha256_final(&ctx, digest);
        to_hex_upper(digest, 32, out_hex);
    } else if (method == METHOD_SHA384) {
        sha384_ctx ctx;
        sha384_init(&ctx);
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            sha384_update(&ctx, (const uint8 *)buffer, bytes_read);
        }
        uint8_t digest[48];
        sha384_final(&ctx, digest);
        to_hex_upper(digest, 48, out_hex);
    } else if (method == METHOD_SHA512) {
        sha512_ctx ctx;
        sha512_init(&ctx);
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            sha512_update(&ctx, (const uint8 *)buffer, bytes_read);
        }
        uint8_t digest[64];
        sha512_final(&ctx, digest);
        to_hex_upper(digest, 64, out_hex);
    } else {
        return -2;
    }
    return 0;
}

static int hash_sha3(FILE *f, ChecksumMethod method, char *out_hex) {
    sha3_ctx_t ctx;
    int mdlen = 0;
    if (method == METHOD_SHA3_256) mdlen = 32;
    else if (method == METHOD_SHA3_384) mdlen = 48;
    else if (method == METHOD_SHA3_512) mdlen = 64;
    else return -2;

    sha3_init(&ctx, mdlen);
    uint8_t buffer[16384];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha3_update(&ctx, buffer, bytes_read);
    }
    uint8_t digest[64];
    sha3_final(digest, &ctx);
    to_hex_upper(digest, mdlen, out_hex);
    return 0;
}

static int hash_blake3(FILE *f, char *out_hex) {
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    uint8_t buffer[16384];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        blake3_hasher_update(&hasher, buffer, bytes_read);
    }
    uint8_t digest[32];
    blake3_hasher_finalize(&hasher, digest, 32);
    to_hex_upper(digest, 32, out_hex);
    return 0;
}

static int hash_xxhash(FILE *f, ChecksumMethod method, char *out_hex) {
    uint8_t buffer[16384];
    size_t bytes_read;

    if (method == METHOD_XXHASH64) {
        XXH64_state_t *state = XXH64_createState();
        if (!state) return -1;
        XXH64_reset(state, 0);
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            XXH64_update(state, buffer, bytes_read);
        }
        uint64_t hash_val = XXH64_digest(state);
        XXH64_freeState(state);
        uint64_to_hex_be(hash_val, out_hex);
    } else if (method == METHOD_XXHASH3) {
        XXH3_state_t *state = XXH3_createState();
        if (!state) return -1;
        XXH3_64bits_reset(state);
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            XXH3_64bits_update(state, buffer, bytes_read);
        }
        uint64_t hash_val = XXH3_64bits_digest(state);
        XXH3_freeState(state);
        uint64_to_hex_be(hash_val, out_hex);
    } else {
        return -2;
    }
    return 0;
}

int compute_hash(const char *path, ChecksumMethod method, char *out_hex) {
    if (!path || !out_hex) return -1;
    if (method < 0 || method >= METHOD_COUNT) return -2;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    int res = 0;
    switch (method) {
        case METHOD_CRC32:
            res = hash_crc32(f, out_hex);
            break;
        case METHOD_MD5:
            res = hash_md5(f, out_hex);
            break;
        case METHOD_SHA1:
            res = hash_sha1(f, out_hex);
            break;
        case METHOD_SHA256:
        case METHOD_SHA384:
        case METHOD_SHA512:
            res = hash_sha2(f, method, out_hex);
            break;
        case METHOD_SHA3_256:
        case METHOD_SHA3_384:
        case METHOD_SHA3_512:
            res = hash_sha3(f, method, out_hex);
            break;
        case METHOD_BLAKE3:
            res = hash_blake3(f, out_hex);
            break;
        case METHOD_XXHASH64:
        case METHOD_XXHASH3:
            res = hash_xxhash(f, method, out_hex);
            break;
        default:
            res = -2;
            break;
    }

    fclose(f);
    return res;
}
