#include "crc32.h"
#include <stdbool.h>

static uint32_t s_crc32_table[256];
static volatile bool s_crc32_initialized = false;

void crc32_init(void) {
    if (s_crc32_initialized) return;

    uint32_t poly = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc = crc >> 1;
            }
        }
        s_crc32_table[i] = crc;
    }
    s_crc32_initialized = true;
}

uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len) {
    if (!s_crc32_initialized) {
        crc32_init();
    }
    for (size_t i = 0; i < len; i++) {
        crc = (crc >> 8) ^ s_crc32_table[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}
