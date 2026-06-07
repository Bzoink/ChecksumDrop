#include "checksum_method.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

const MethodInfo METHOD_TABLE[METHOD_COUNT] = {
    [METHOD_CRC32]    = { "CRC32",    "sfv",       8 },
    [METHOD_MD5]      = { "MD5",      "md5",      32 },
    [METHOD_SHA1]     = { "SHA1",     "sha1",     40 },
    [METHOD_SHA256]   = { "SHA256",   "sha256",   64 },
    [METHOD_SHA384]   = { "SHA384",   "sha384",   96 },
    [METHOD_SHA512]   = { "SHA512",   "sha512",  128 },
    [METHOD_SHA3_256] = { "SHA3-256", "sha3-256", 64 },
    [METHOD_SHA3_384] = { "SHA3-384", "sha3-384", 96 },
    [METHOD_SHA3_512] = { "SHA3-512", "sha3-512",128 },
    [METHOD_BLAKE3]   = { "BLAKE3",   "blake3",   64 },
    [METHOD_XXHASH64] = { "xxHash64", "xxh64",    16 },
    [METHOD_XXHASH3]  = { "xxHash3",  "xxh3",     16 }
};

const char *checksum_method_to_display_name(ChecksumMethod method) {
    if (method < 0 || method >= METHOD_COUNT) return "UNKNOWN";
    return METHOD_TABLE[method].display_name;
}

const char *checksum_method_to_token(ChecksumMethod method) {
    if (method < 0 || method >= METHOD_COUNT) return "unknown";
    return METHOD_TABLE[method].token;
}

int checksum_method_get_hash_length(ChecksumMethod method) {
    if (method < 0 || method >= METHOD_COUNT) return 0;
    return METHOD_TABLE[method].hex_length;
}

void checksum_method_get_output_filename(ChecksumMethod method, char *out_buf, int out_size) {
    if (method == METHOD_CRC32) {
        snprintf(out_buf, out_size, "checksum.sfv");
    } else {
        snprintf(out_buf, out_size, "checksum.%s.txt", checksum_method_to_token(method));
    }
}

static int case_insensitive_compare(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return 0;
        }
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

int checksum_method_try_parse_digest_path(const char *path, ChecksumMethod *out_method) {
    if (!path) return 0;

    /* Get filename part */
    const char *filename = strrchr(path, '/');
    const char *win_slash = strrchr(path, '\\');
    if (win_slash && (!filename || win_slash > filename)) {
        filename = win_slash;
    }
    if (filename) {
        filename++; /* skip slash */
    } else {
        filename = path;
    }

    if (strlen(filename) == 0) return 0;

    /* Copy filename for tokenization */
    char file_copy[1024];
    snprintf(file_copy, sizeof(file_copy), "%s", filename);

    /* Array of boolean flags to see which methods matched */
    int matched_methods[METHOD_COUNT] = {0};
    int total_unique_matches = 0;

    /* Custom tokenization by '.' to handle trim/empty check */
    char *token = strtok(file_copy, ".");
    while (token != NULL) {
        /* Trim spaces from token */
        while (*token && isspace((unsigned char)*token)) token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace((unsigned char)*end)) {
            *end = '\0';
            end--;
        }

        if (strlen(token) > 0) {
            for (int i = 0; i < METHOD_COUNT; i++) {
                if (case_insensitive_compare(token, METHOD_TABLE[i].token)) {
                    if (!matched_methods[i]) {
                        matched_methods[i] = 1;
                        total_unique_matches++;
                    }
                }
            }
        }
        token = strtok(NULL, ".");
    }

    /* We must have exactly 1 unique method match */
    if (total_unique_matches == 1) {
        for (int i = 0; i < METHOD_COUNT; i++) {
            if (matched_methods[i]) {
                if (out_method) *out_method = (ChecksumMethod)i;
                return 1;
            }
        }
    }

    return 0;
}

int checksum_method_try_parse_setting(const char *value, ChecksumMethod *out_method) {
    if (!value) return 0;

    /* Trim leading and trailing spaces */
    while (*value && isspace((unsigned char)*value)) value++;
    int len = (int)strlen(value);
    while (len > 0 && isspace((unsigned char)value[len - 1])) len--;

    if (len == 0) return 0;

    /* Prepare normalized version: no hyphens, case-insensitive check */
    char norm[256];
    int norm_idx = 0;
    for (int i = 0; i < len && norm_idx < 255; i++) {
        if (value[i] != '-') {
            norm[norm_idx++] = tolower((unsigned char)value[i]);
        }
    }
    norm[norm_idx] = '\0';

    for (int i = 0; i < METHOD_COUNT; i++) {
        /* Compare normalized input with METHOD_TABLE[i].token (with and without hyphen) */
        char m_token_norm[256];
        int m_idx = 0;
        const char *t = METHOD_TABLE[i].token;
        while (*t && m_idx < 255) {
            if (*t != '-') {
                m_token_norm[m_idx++] = tolower((unsigned char)*t);
            }
            t++;
        }
        m_token_norm[m_idx] = '\0';

        if (strcmp(norm, m_token_norm) == 0) {
            if (out_method) *out_method = (ChecksumMethod)i;
            return 1;
        }

        /* Also check against the display name without hyphens */
        m_idx = 0;
        t = METHOD_TABLE[i].display_name;
        while (*t && m_idx < 255) {
            if (*t != '-') {
                m_token_norm[m_idx++] = tolower((unsigned char)*t);
            }
            t++;
        }
        m_token_norm[m_idx] = '\0';

        if (strcmp(norm, m_token_norm) == 0) {
            if (out_method) *out_method = (ChecksumMethod)i;
            return 1;
        }
    }

    return 0;
}
