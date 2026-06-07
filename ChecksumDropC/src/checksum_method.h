#pragma once

typedef enum {
    METHOD_CRC32,
    METHOD_MD5,
    METHOD_SHA1,
    METHOD_SHA256,
    METHOD_SHA384,
    METHOD_SHA512,
    METHOD_SHA3_256,
    METHOD_SHA3_384,
    METHOD_SHA3_512,
    METHOD_BLAKE3,
    METHOD_XXHASH64,
    METHOD_XXHASH3,
    METHOD_COUNT
} ChecksumMethod;

typedef struct {
    const char *display_name;   /* e.g., "SHA3-256" */
    const char *token;          /* e.g., "sha3-256" */
    int         hex_length;     /* e.g., 64 */
} MethodInfo;

extern const MethodInfo METHOD_TABLE[METHOD_COUNT];

/* Helper functions matching ChecksumMethodExtensions */
const char *checksum_method_to_display_name(ChecksumMethod method);
const char *checksum_method_to_token(ChecksumMethod method);
int checksum_method_get_hash_length(ChecksumMethod method);
void checksum_method_get_output_filename(ChecksumMethod method, char *out_buf, int out_size);

/* Tries to parse the method from a digest filepath. Returns 1 if successful, 0 otherwise. */
int checksum_method_try_parse_digest_path(const char *path, ChecksumMethod *out_method);

/* Tries to parse the method from a configuration string. Returns 1 if successful, 0 otherwise. */
int checksum_method_try_parse_setting(const char *value, ChecksumMethod *out_method);
