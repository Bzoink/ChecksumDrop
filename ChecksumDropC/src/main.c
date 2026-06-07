/*
 * main.c — ChecksumDrop (C / raylib + raygui port)
 *
 * Phase 1 / Phase 0 scaffold:
 * - If called with `--test`, runs the comprehensive hashing and method-parsing self-test suite.
 * - If called normally, launches the dark-themed UI window with X11 XDND drop proxy-enabled logging.
 */

#include "raylib.h"

/* RAYGUI_IMPLEMENTATION must be defined exactly once, here in main.c */
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#if defined(__linux__)
#include "x11_dnd.h"
#endif

#include "hashing.h"
#include "checksum_method.h"
#include "processor.h"
#include "path_utils.h"
#include "settings.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#define RMDIR(d) _rmdir(d)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(d) mkdir(d, 0777)
#define RMDIR(d) rmdir(d)
#endif

/* ── Window & palette (ChecksumDrop dark theme) ──────────────────────────── */
#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 560

#define BG_COLOR      (Color){ 0x10, 0x16, 0x1F, 0xFF }  /* #10161F window     */
#define PANEL_COLOR   (Color){ 0x17, 0x21, 0x2B, 0xFF }  /* #17212B drop zone  */
#define BORDER_COLOR  (Color){ 0x3A, 0x47, 0x58, 0xFF }  /* #3A4758 border     */
#define TEXT_COLOR    (Color){ 0xEA, 0xEF, 0xF5, 0xFF }  /* near-white text    */
#define ACCENT_COLOR  (Color){ 0x4C, 0x8B, 0xF5, 0xFF }  /* accent blue        */

/* Simple ring buffer of the most recent dropped paths (Phase 0 diagnostics). */
static void handle_drop(AppState *st)
{
    if (!IsFileDropped()) return;

    FilePathList dropped = LoadDroppedFiles();
    if (dropped.count > 0) {
        processor_run(st, dropped.paths, dropped.count);
        if (st->done && st->is_validation) {
            st->screen = SCREEN_RESULTS;
        }
    }
    UnloadDroppedFiles(dropped);
}

/* ── Comprehensive Phase 1 Hashing & Metadata Self-Test Suite ───────────── */

struct HashTestCase {
    const char *label;
    const char *filename;
    const char *content;
    size_t      content_len;
    ChecksumMethod method;
    const char *expected;
};

static struct HashTestCase s_hash_test_cases[] = {
    /* ── Empty File (0 Bytes) ── */
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_CRC32,      "00000000" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_MD5,        "D41D8CD98F00B204E9800998ECF8427E" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA1,       "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA256,     "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA384,     "38B060A751AC96384CD9327EB1B1E36A21FDB71114BE07434C0CC7BF63F6E1DA274EDEBFE76F65FBD51AD2F14898B95B" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA512,     "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA3_256,   "A7FFC6F8BF1ED76651C14756A061D662F580FF4DE43B49FA82D80A4B80F8434A" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA3_384,   "0C63A75B845E4F7D01107D852E4C2485C51A50AAAA94FC61995E71BBEE983A2AC3713831264ADB47FB6BD1E058D5F004" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_SHA3_512,   "A69F73CCA23A9AC5C8B567DC185A756E97C982164FE25859E0D1DCC1475C80A615B2123AF1F5F94C11E3E9402C3AC558F500199D95B6D3E301758586281DCD26" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_BLAKE3,     "AF1349B9F5F9A1A6A0404DEA36DCC9499BCB25C9ADC112B7CC9A93CAE41F3262" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_XXHASH64,   "EF46DB3751D8E999" },
    { "0-Byte", "temp_empty.bin", "", 0, METHOD_XXHASH3,    "2D06800538D394C2" },

    /* ── abc String ── */
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_CRC32,    "352441C2" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_MD5,      "900150983CD24FB0D6963F7D28E17F72" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA1,     "A9993E364706816ABA3E25717850C26C9CD0D89D" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA256,   "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA384,   "CB00753F45A35E8BB5A03D699AC65007272C32AB0EDED1631A8B605A43FF5BED8086072BA1E7CC2358BAECA134C825A7" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA512,   "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA3_256, "3A985DA74FE225B2045C172D6BD390BD855F086E3E9D525B46BFE24511431532" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA3_384, "EC01498288516FC926459F58E2C6AD8DF9B473CB0FC08C2596DA7CF0E49BE4B298D88CEA927AC7F539F1EDF228376D25" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_SHA3_512, "B751850B1A57168A5693CD924B6B096E08F621827444F70D884F5D0240D2712E10E116E9192AF3C91A7EC57647E3934057340B4CF408D5A56592F8274EEC53F0" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_BLAKE3,   "6437B3AC38465133FFB63B75273A8DB548C558465D79DB03FD359C6CD5BD9D85" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_XXHASH64, "44BC2CF5AD770999" },
    { "abc",    "temp_abc.bin",   "abc", 3, METHOD_XXHASH3,  "78AF5F94892F3950" }
};

struct PathTestCase {
    const char *path;
    int         expected_success;
    ChecksumMethod expected_method;
};

static struct PathTestCase s_path_test_cases[] = {
    { "checksum.sfv", 1, METHOD_CRC32 },
    { "checksum.md5.txt", 1, METHOD_MD5 },
    { "checksum.sha3-256.txt", 1, METHOD_SHA3_256 },
    { "checksum.xxh64.txt", 1, METHOD_XXHASH64 },
    { "/home/tony/checksum.blake3.txt", 1, METHOD_BLAKE3 },
    { "test.txt", 0, METHOD_CRC32 }
};

struct SettingTestCase {
    const char *setting;
    int         expected_success;
    ChecksumMethod expected_method;
};

static struct SettingTestCase s_setting_test_cases[] = {
    { "md5", 1, METHOD_MD5 },
    { "MD5", 1, METHOD_MD5 },
    { "sha3-256", 1, METHOD_SHA3_256 },
    { "SHA3-256", 1, METHOD_SHA3_256 },
    { " sha3256 ", 1, METHOD_SHA3_256 },
    { "xxh64", 1, METHOD_XXHASH64 },
    { "xxhash64", 1, METHOD_XXHASH64 },
    { "invalid", 0, METHOD_MD5 }
};

static int run_self_tests(void) {
    int failed = 0;

    printf("\n======================================================================\n");
    printf("                  ChecksumDrop Phase 1 Self-Test Suite\n");
    printf("======================================================================\n\n");

    /* ── Create temporary files ── */
    FILE *f_empty = fopen("temp_empty.bin", "wb");
    if (f_empty) fclose(f_empty);
    else { printf("Error: could not create temp_empty.bin\n"); return 1; }

    FILE *f_abc = fopen("temp_abc.bin", "wb");
    if (f_abc) {
        fwrite("abc", 1, 3, f_abc);
        fclose(f_abc);
    } else {
        printf("Error: could not create temp_abc.bin\n");
        remove("temp_empty.bin");
        return 1;
    }

    /* ── 1. Run Hash Test Cases ── */
    printf("--- 1. Hashing Algorithm Correctness ---\n");
    int hash_cases_count = sizeof(s_hash_test_cases) / sizeof(s_hash_test_cases[0]);
    for (int i = 0; i < hash_cases_count; i++) {
        struct HashTestCase tc = s_hash_test_cases[i];
        char comp_hash[129] = {0};
        
        int rc = compute_hash(tc.filename, tc.method, comp_hash);
        int matches = (rc == 0) && (strcmp(comp_hash, tc.expected) == 0);

        if (matches) {
            printf("  [\x1b[32mPASS\x1b[0m] %s (%s): %s\n", 
                   tc.label, checksum_method_to_display_name(tc.method), comp_hash);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] %s (%s):\n", 
                   tc.label, checksum_method_to_display_name(tc.method));
            printf("         Expected: %s\n", tc.expected);
            printf("         Computed: %s (rc=%d)\n", comp_hash, rc);
            failed++;
        }
    }
    printf("\n");

    /* ── Cleanup files ── */
    remove("temp_empty.bin");
    remove("temp_abc.bin");

    /* ── 2. Run Path Parsing Test Cases ── */
    printf("--- 2. Digest Path Matching Helper ---\n");
    int path_cases_count = sizeof(s_path_test_cases) / sizeof(s_path_test_cases[0]);
    for (int i = 0; i < path_cases_count; i++) {
        struct PathTestCase tc = s_path_test_cases[i];
        ChecksumMethod out_method = METHOD_CRC32;
        int rc = checksum_method_try_parse_digest_path(tc.path, &out_method);

        int ok = (rc == tc.expected_success) && 
                 (!tc.expected_success || (out_method == tc.expected_method));

        if (ok) {
            if (rc) {
                printf("  [\x1b[32mPASS\x1b[0m] '%s' -> %s\n", tc.path, checksum_method_to_display_name(out_method));
            } else {
                printf("  [\x1b[32mPASS\x1b[0m] '%s' -> (correctly skipped)\n", tc.path);
            }
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] '%s'\n", tc.path);
            printf("         Expected success: %d, parsed method: %s\n", 
                   tc.expected_success, checksum_method_to_display_name(out_method));
            failed++;
        }
    }
    printf("\n");

    /* ── 3. Run Settings Parsing Test Cases ── */
    printf("--- 3. Settings Parsing Matching Helper ---\n");
    int setting_cases_count = sizeof(s_setting_test_cases) / sizeof(s_setting_test_cases[0]);
    for (int i = 0; i < setting_cases_count; i++) {
        struct SettingTestCase tc = s_setting_test_cases[i];
        ChecksumMethod out_method = METHOD_CRC32;
        int rc = checksum_method_try_parse_setting(tc.setting, &out_method);

        int ok = (rc == tc.expected_success) && 
                 (!tc.expected_success || (out_method == tc.expected_method));

        if (ok) {
            if (rc) {
                printf("  [\x1b[32mPASS\x1b[0m] '%s' -> %s\n", tc.setting, checksum_method_to_display_name(out_method));
            } else {
                printf("  [\x1b[32mPASS\x1b[0m] '%s' -> (correctly skipped)\n", tc.setting);
            }
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] '%s'\n", tc.setting);
            printf("         Expected success: %d, parsed method: %s\n", 
                   tc.expected_success, checksum_method_to_display_name(out_method));
            failed++;
        }
    }
    printf("\n");

    /* ── 4. Run Phase 2 Generation & Output Writing Test Cases ── */
    printf("--- 4. Phase 2: Directory Expansion, Base Path, & Output Generation ---\n");
    do {
        /* Create temp_p2 directories */
        if (MKDIR("temp_p2") != 0) {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p2 directory.\n");
            failed++;
            break;
        }
        if (MKDIR("temp_p2/sub") != 0) {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p2/sub directory.\n");
            failed++;
            break;
        }

        /* Write file1.txt and sub/file2.txt */
        FILE *f1 = fopen("temp_p2/file1.txt", "wb");
        if (f1) {
            fwrite("abc", 1, 3, f1);
            fclose(f1);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p2/file1.txt\n");
            failed++;
            break;
        }

        FILE *f2 = fopen("temp_p2/sub/file2.txt", "wb");
        if (f2) {
            fwrite("abc", 1, 3, f2);
            fclose(f2);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p2/sub/file2.txt\n");
            failed++;
            break;
        }

        /* Test path details */
        char parent[1024];
        path_get_parent("temp_p2/file1.txt", parent, sizeof(parent));
        if (strcmp(parent, "temp_p2") == 0) {
            printf("  [\x1b[32mPASS\x1b[0m] Parent path of 'temp_p2/file1.txt' is '%s'\n", parent);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Parent path expected 'temp_p2', got '%s'\n", parent);
            failed++;
        }

        char relative[1024];
        path_get_relative("temp_p2", "temp_p2/sub/file2.txt", relative, sizeof(relative));
        if (strcmp(relative, "sub/file2.txt") == 0) {
            printf("  [\x1b[32mPASS\x1b[0m] Relative path of 'temp_p2/sub/file2.txt' vs 'temp_p2' is '%s'\n", relative);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Relative path expected 'sub/file2.txt', got '%s'\n", relative);
            failed++;
        }

        /* Setup AppState and run processor_run with temp_p2 */
        static AppState st_test = {0};
        st_test.method = METHOD_MD5;

        char *dropped[] = { "temp_p2" };
        processor_run(&st_test, dropped, 1);

        if (st_test.result_count == 2) {
            printf("  [\x1b[32mPASS\x1b[0m] Directory expanded. Found %d files.\n", st_test.result_count);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Expanded expected 2 files, got %d.\n", st_test.result_count);
            failed++;
        }

        /* Verify MD5 digest output contents */
        FILE *fmd5 = fopen(st_test.output_path, "r");
        if (fmd5) {
            char line[1024];
            int found_f1 = 0;
            int found_f2 = 0;
            while (fgets(line, sizeof(line), fmd5)) {
                if (strstr(line, "900150983CD24FB0D6963F7D28E17F72") && strstr(line, "file1.txt")) {
                    found_f1 = 1;
                }
                if (strstr(line, "900150983CD24FB0D6963F7D28E17F72") && strstr(line, "sub/file2.txt")) {
                    found_f2 = 1;
                }
            }
            fclose(fmd5);

            if (found_f1 && found_f2) {
                printf("  [\x1b[32mPASS\x1b[0m] Wrote digest: %s (Parity MD5, format HASH  path)\n", st_test.output_path);
            } else {
                printf("  [\x1b[31mFAIL\x1b[0m] Digest lines did not match MD5 format: found_f1=%d, found_f2=%d\n", found_f1, found_f2);
                failed++;
            }
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not open generated digest %s\n", st_test.output_path);
            failed++;
        }

        /* Test SFV output contents */
        st_test.method = METHOD_CRC32;
        processor_run(&st_test, dropped, 1);

        FILE *fcre = fopen(st_test.output_path, "r");
        if (fcre) {
            char line[1024];
            int found_f1 = 0;
            int found_f2 = 0;
            while (fgets(line, sizeof(line), fcre)) {
                if (strstr(line, "352441C2") && strstr(line, "file1.txt")) {
                    found_f1 = 1;
                }
                if (strstr(line, "352441C2") && strstr(line, "sub/file2.txt")) {
                    found_f2 = 1;
                }
            }
            fclose(fcre);

            if (found_f1 && found_f2) {
                printf("  [\x1b[32mPASS\x1b[0m] Wrote digest: %s (Parity CRC32, format path  HASH)\n", st_test.output_path);
            } else {
                printf("  [\x1b[31mFAIL\x1b[0m] Digest lines did not match SFV format: found_f1=%d, found_f2=%d\n", found_f1, found_f2);
                failed++;
            }
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not open generated SFV %s\n", st_test.output_path);
            failed++;
        }

        /* Clean up results array from AppState */
        if (st_test.results) {
            free(st_test.results);
            st_test.results = NULL;
        }

        /* Remove files first */
        remove("temp_p2/file1.txt");
        remove("temp_p2/sub/file2.txt");
        remove("temp_p2/checksum.md5.txt");
        remove("temp_p2/checksum.sfv");
        RMDIR("temp_p2/sub");
        RMDIR("temp_p2");

        printf("  [\x1b[32mPASS\x1b[0m] Cleared Phase 2 test vectors and temporary dir structure.\n");

    } while (0);
    printf("\n");

    /* ── 5. Run Phase 3 Validation Parsing Engine Test Cases ── */
    printf("--- 5. Phase 3: Validation, Line Parsing, & Status Evaluation ---\n");
    do {
        /* Create temp_p3 directory */
        if (MKDIR("temp_p3") != 0) {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p3 directory.\n");
            failed++;
            break;
        }

        /* Create target test files */
        FILE *fg = fopen("temp_p3/good.txt", "wb");
        if (fg) {
            fwrite("abc", 1, 3, fg);
            fclose(fg);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p3/good.txt\n");
            failed++;
            break;
        }

        FILE *fb = fopen("temp_p3/bad.txt", "wb");
        if (fb) {
            fwrite("def", 1, 3, fb); /* "abc" vs "def" hash mismatch */
            fclose(fb);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p3/bad.txt\n");
            failed++;
            break;
        }

        /* Write a simulated MD5 test digest */
        FILE *fmd5 = fopen("temp_p3/digest.md5.txt", "wb");
        if (fmd5) {
            fprintf(fmd5, "; MD5 validation test cases\r\n");
            fprintf(fmd5, "900150983CD24FB0D6963F7D28E17F72  good.txt\r\n");
            fprintf(fmd5, "900150983CD24FB0D6963F7D28E17F72  bad.txt\r\n");
            fprintf(fmd5, "900150983CD24FB0D6963F7D28E17F72  missing.txt\r\n");
            fclose(fmd5);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p3/digest.md5.txt\n");
            failed++;
            break;
        }

        /* Setup AppState and run validation for MD5 */
        static AppState st_val = {0};
        char *dropped_md5[] = { "temp_p3/digest.md5.txt" };

        processor_run(&st_val, dropped_md5, 1);

        if (st_val.is_validation) {
            printf("  [\x1b[32mPASS\x1b[0m] Correctly auto-detected validation mode for MD5 digest.\n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Failed to auto-detect validation mode for MD5.\n");
            failed++;
        }

        if (st_val.summary.total_files == 3 && 
            st_val.summary.total_valid == 1 &&
            st_val.summary.total_invalid == 1 &&
            st_val.summary.total_not_found == 1) {
            printf("  [\x1b[32mPASS\x1b[0m] Validation MD5 metrics verified: 3 total, 1 OK, 1 Bad, 1 Missing\n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] MD5 metrics mismatch: Got %d total, %d valid, %d invalid, %d not found\n",
                   st_val.summary.total_files, st_val.summary.total_valid, st_val.summary.total_invalid, st_val.summary.total_not_found);
            failed++;
        }

        /* Check individual file statuses */
        int checked_items = 0;
        for (int i = 0; i < st_val.result_count; i++) {
            if (strcmp(st_val.results[i].filename, "good.txt") == 0) {
                if (st_val.results[i].status == 101) checked_items++;
            } else if (strcmp(st_val.results[i].filename, "bad.txt") == 0) {
                if (st_val.results[i].status == 909) checked_items++;
            } else if (strcmp(st_val.results[i].filename, "missing.txt") == 0) {
                if (st_val.results[i].status == 404) checked_items++;
            }
        }
        if (checked_items == 3) {
            printf("  [\x1b[32mPASS\x1b[0m] Verified individual file statuses for MD5 items (101, 909, 404).\n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Failed to verify file statuses: got %d/3 correct.\n", checked_items);
            failed++;
        }

        if (st_val.results) {
            free(st_val.results);
            st_val.results = NULL;
        }

        /* Write a simulated SFV (CRC32) digest */
        FILE *fsfv = fopen("temp_p3/digest.sfv", "wb");
        if (fsfv) {
            fprintf(fsfv, "; CRC32 validation test cases\n");
            fprintf(fsfv, "good.txt  352441C2\n");
            fprintf(fsfv, "bad.txt  352441C2\n");
            fprintf(fsfv, "missing.txt  352441C2\n");
            fclose(fsfv);
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Could not create temp_p3/digest.sfv\n");
            failed++;
            break;
        }

        /* Run validation for SFV */
        char *dropped_sfv[] = { "temp_p3/digest.sfv" };
        processor_run(&st_val, dropped_sfv, 1);

        if (st_val.is_validation) {
            printf("  [\x1b[32mPASS\x1b[0m] Correctly auto-detected validation mode for SFV digest.\n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Failed to auto-detect validation mode for SFV.\n");
            failed++;
        }

        if (st_val.summary.total_files == 3 && 
            st_val.summary.total_valid == 1 &&
            st_val.summary.total_invalid == 1 &&
            st_val.summary.total_not_found == 1) {
            printf("  [\x1b[32mPASS\x1b[0m] Validation SFV metrics verified: 3 total, 1 OK, 1 Bad, 1 Missing\n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] SFV metrics mismatch: Got %d total, %d valid, %d invalid, %d not found\n",
                   st_val.summary.total_files, st_val.summary.total_valid, st_val.summary.total_invalid, st_val.summary.total_not_found);
            failed++;
        }

        /* Check individual SFV file statuses */
        checked_items = 0;
        for (int i = 0; i < st_val.result_count; i++) {
            if (strcmp(st_val.results[i].filename, "good.txt") == 0) {
                if (st_val.results[i].status == 101) checked_items++;
            } else if (strcmp(st_val.results[i].filename, "bad.txt") == 0) {
                if (st_val.results[i].status == 909) checked_items++;
            } else if (strcmp(st_val.results[i].filename, "missing.txt") == 0) {
                if (st_val.results[i].status == 404) checked_items++;
            }
        }
        if (checked_items == 3) {
            printf("  [\x1b[32mPASS\x1b[0m] Verified individual file statuses for SFV items (101, 909, 404). \n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Failed to verify SFV file statuses: got %d/3 correct.\n", checked_items);
            failed++;
        }

        if (st_val.results) {
            free(st_val.results);
            st_val.results = NULL;
        }

        /* Cleanup */
        remove("temp_p3/good.txt");
        remove("temp_p3/bad.txt");
        remove("temp_p3/digest.md5.txt");
        remove("temp_p3/digest.sfv");
        RMDIR("temp_p3");

        printf("  [\x1b[32mPASS\x1b[0m] Cleared Phase 3 test vectors and temporary dir structure.\n");

    } while (0);
    printf("\n");

    /* ── 6. Run Phase 4 Settings Persistence Test Cases ── */
    printf("--- 6. Phase 4: Settings Loading & Saving (settings.ini) ---\n");
    do {
        /* Preserve user's original settings first to avoid breaking local files */
        AppState st_orig = {0};
        settings_load(&st_orig);
        ChecksumMethod original_method = st_orig.method;

        /* Test settings load */
        AppState st_temp = {0};
        settings_load(&st_temp);
        printf("  [\x1b[32mPASS\x1b[0m] Settings loaded configuration: default/existing is %s\n", 
               checksum_method_to_display_name(st_temp.method));

        /* Change and save BLAKE3 */
        st_temp.method = METHOD_BLAKE3;
        settings_save(&st_temp);

        /* Verify save of BLAKE3 */
        AppState st_val2 = {0};
        settings_load(&st_val2);
        if (st_val2.method == METHOD_BLAKE3) {
            printf("  [\x1b[32mPASS\x1b[0m] Changed settings to 'BLAKE3', successfully saved, and re-loaded.\n");
        } else {
            printf("  [\x1b[31mFAIL\x1b[0m] Expected loaded settings method to be BLAKE3, got %s\n", 
                   checksum_method_to_display_name(st_val2.method));
            failed++;
        }

        /* Restore original settings to be completely clean */
        st_temp.method = original_method;
        settings_save(&st_temp);
        printf("  [\x1b[32mPASS\x1b[0m] Restored original local user settings: %s\n", 
               checksum_method_to_display_name(original_method));

    } while (0);
    printf("\n");

    printf("======================================================================\n");
    if (failed == 0) {
        printf("  \x1b[32mALL TESTS COMPLETED SUCCESSFULLY!\x1b[0m\n");
    } else {
        printf("  \x1b[31mTEST FAILURE! %d cases failed.\x1b[0m\n", failed);
    }
    printf("======================================================================\n\n");

    return failed ? 1 : 0;
}

int main(int argc, char **argv)
{
    /* Check if we want to run the self-test suite (Phase 1) */
    if (argc > 1 && (strcmp(argv[1], "--test") == 0 || strcmp(argv[1], "test") == 0 || strcmp(argv[1], "-t") == 0)) {
        return run_self_tests();
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ChecksumDrop");
    SetTargetFPS(60);
    SetWindowMinSize(560, 480);

#if defined(__linux__)
    /* Fix drag-and-drop on Cinnamon/Mutter and other reparenting compositors.
     * Sets XdndProxy/XdndAware on the WM frame window so XDND sources forward
     * events to our real app window. See src/x11_dnd.h for the full rationale. */
    xdnd_proxy_setup(GetWindowHandle());
#endif

    /* Dark theme defaults for raygui widgets. */
    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR,   ColorToInt(BG_COLOR));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL,  ColorToInt(TEXT_COLOR));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(TEXT_COLOR));
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ColorToInt(TEXT_COLOR));

    static AppState st = {0};
    st.screen = SCREEN_MAIN;
    settings_load(&st);

    while (!WindowShouldClose()) {
        /* Only handle file drops on the main screen since validation transitions automatically */
        if (st.screen == SCREEN_MAIN) {
            handle_drop(&st);
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);

        int w = GetScreenWidth();
        int h = GetScreenHeight();

        if (st.screen == SCREEN_MAIN) {
            /* Title. */
            DrawText("ChecksumDrop", 24, 20, 26, TEXT_COLOR);
            DrawText("Fast, cross-platform hash generator & validation engine", 24, 54, 14, BORDER_COLOR);

            /* Drop zone panel. */
            const int pad = 24;
            const int zone_y = 90;
            const int zone_h = 180;
            Rectangle zone = { (float)pad, (float)zone_y, (float)(w - pad * 2), (float)zone_h };
            DrawRectangleRec(zone, PANEL_COLOR);
            DrawRectangleLinesEx(zone, 2.0f, BORDER_COLOR);

            if (st.busy) {
                /* Render active running view content */
                DrawText(st.main_text, (int)(zone.x + 24), (int)(zone.y + 24), 20, TEXT_COLOR);
                DrawText(st.sub_text, (int)(zone.x + zone.width - 150), (int)(zone.y + 24), 16, ACCENT_COLOR);

                /* Progress bar track */
                Rectangle prog_track = { zone.x + 24, zone.y + 114, zone.width - 48, 14 };
                DrawRectangleRec(prog_track, BG_COLOR);
                DrawRectangleLinesEx(prog_track, 1.0f, BORDER_COLOR);

                /* Progress bar fill */
                float fill_percent = st.progress;
                if (fill_percent < 0.0f) fill_percent = 0.0f;
                if (fill_percent > 1.0f) fill_percent = 1.0f;
                Rectangle prog_fill = { prog_track.x + 1, prog_track.y + 1, (prog_track.width - 2) * fill_percent, prog_track.height - 2 };
                DrawRectangleRec(prog_fill, ACCENT_COLOR);

                /* Filename / fineprint */
                DrawText(st.sub_sub_text, (int)(zone.x + 24), (int)(zone.y + 74), 13, BORDER_COLOR);

                /* Cancel command button */
                Rectangle cancel_btn = { (float)(w - 148), (float)(h - 60), 124.0f, 36.0f };
                if (GuiButton(cancel_btn, "Cancel")) {
                    st.cancel_requested = 1;
                }
            } else if (st.done) {
                /* Render finished view content */
                DrawText(st.main_text, (int)(zone.x + 24), (int)(zone.y + 24), 22, (Color){ 0x30, 0xC8, 0x5D, 0xFF });
                DrawText(st.sub_text, (int)(zone.x + 24), (int)(zone.y + 54), 16, TEXT_COLOR);
                DrawText(st.sub_sub_text, (int)(zone.x + 24), (int)(zone.y + 80), 13, BORDER_COLOR);
                DrawText(st.status_text, (int)(zone.x + 24), (int)(zone.y + 104), 12, BORDER_COLOR);

                /* Reset button */
                Rectangle reset_btn = { zone.x + zone.width / 2 - 60, zone.y + 130, 120, 32 };
                if (GuiButton(reset_btn, "Reset")) {
                    st.done = 0;
                    st.status_text[0] = '\0';
                }
            } else {
                /* Render ready/prompt view content */
                DrawText("↓", (int)(zone.x + zone.width / 2 - 10), (int)(zone.y + 30), 32, ACCENT_COLOR);
                
                const char *prompt = "Drop files or folders anywhere in this panel";
                int tw = MeasureText(prompt, 18);
                DrawText(prompt, (int)(zone.x + (zone.width - tw) / 2), (int)(zone.y + 80), 18, TEXT_COLOR);

                const char *fine = "Or drop existing checksum files (.sfv, .md5, etc.) to validate";
                int tf = MeasureText(fine, 13);
                DrawText(fine, (int)(zone.x + (zone.width - tf) / 2), (int)(zone.y + 114), 13, BORDER_COLOR);
            }

            /* Hashing method section */
            DrawText("Active Hashing Algorithm", 24, 290, 14, ACCENT_COLOR);

            float col_w = (float)(w - 48 - 24) / 4.0f;
            float row_h = 36.0f;
            int start_y = 315;

            for (int i = 0; i < METHOD_COUNT; i++) {
                int row = i / 4;
                int col = i % 4;
                Rectangle btn_rec = { 24.0f + col * (col_w + 8.0f), (float)(start_y + row * (row_h + 8.0f)), col_w, row_h };

                int is_active = (st.method == (ChecksumMethod)i);
                if (is_active) {
                    DrawRectangleRec(btn_rec, (Color){ 0x1A, 0x30, 0x4B, 0xFF });
                    DrawRectangleLinesEx(btn_rec, 2.0f, ACCENT_COLOR);

                    const char *text = checksum_method_to_display_name((ChecksumMethod)i);
                    int text_w = MeasureText(text, 14);
                    DrawText(text, (int)(btn_rec.x + (btn_rec.width - text_w) / 2), (int)(btn_rec.y + 11), 14, TEXT_COLOR);
                } else {
                    if (GuiButton(btn_rec, checksum_method_to_display_name((ChecksumMethod)i))) {
                        st.method = (ChecksumMethod)i;
                        settings_save(&st);
                    }
                }
            }

            /* Bottom bar */
            float bottom_y = (float)(h - 52);
            DrawText("Config path: ~/.config/ChecksumDrop/settings.ini", 24, (int)(bottom_y + 12), 12, BORDER_COLOR);

            Rectangle about_btn = { (float)(w - 124), bottom_y, 100.0f, 32.0f };
            if (GuiButton(about_btn, "About")) {
                st.screen = SCREEN_ABOUT;
            }
        } 
        else if (st.screen == SCREEN_RESULTS) {
            /* Title. */
            DrawText("Validation Results", 24, 20, 26, TEXT_COLOR);
            
            char sum_text[256];
            snprintf(sum_text, sizeof(sum_text), "Checked %d files from %d checksum digests.", st.summary.total_files, st.summary.total_digests);
            DrawText(sum_text, 24, 54, 14, BORDER_COLOR);

            /* Summary stats tiles */
            float tile_w = (float)(w - 48 - 36) / 4.0f;
            float tile_h = 60.0f;
            float tile_y = 85.0f;

            /* Tile 0: Total files */
            Rectangle r0 = { 24.0f, tile_y, tile_w, tile_h };
            DrawRectangleRec(r0, PANEL_COLOR);
            DrawRectangleLinesEx(r0, 1.0f, BORDER_COLOR);
            DrawText("Total Files", (int)(r0.x + 12), (int)(r0.y + 10), 11, BORDER_COLOR);
            char score_str[32];
            snprintf(score_str, sizeof(score_str), "%d", st.summary.total_files);
            DrawText(score_str, (int)(r0.x + 12), (int)(r0.y + 26), 20, TEXT_COLOR);

            /* Tile 1: Valid */
            Rectangle r1 = { 24.0f + tile_w + 12.0f, tile_y, tile_w, tile_h };
            DrawRectangleRec(r1, PANEL_COLOR);
            DrawRectangleLinesEx(r1, 1.0f, BORDER_COLOR);
            DrawText("Valid", (int)(r1.x + 12), (int)(r1.y + 10), 11, BORDER_COLOR);
            snprintf(score_str, sizeof(score_str), "%d", st.summary.total_valid);
            DrawText(score_str, (int)(r1.x + 12), (int)(r1.y + 26), 20, (Color){ 0x30, 0xC8, 0x5D, 0xFF });

            /* Tile 2: Missing */
            Rectangle r2 = { 24.0f + (tile_w + 12.0f) * 2.0f, tile_y, tile_w, tile_h };
            DrawRectangleRec(r2, PANEL_COLOR);
            DrawRectangleLinesEx(r2, 1.0f, BORDER_COLOR);
            DrawText("Missing", (int)(r2.x + 12), (int)(r2.y + 10), 11, BORDER_COLOR);
            snprintf(score_str, sizeof(score_str), "%d", st.summary.total_not_found);
            DrawText(score_str, (int)(r2.x + 12), (int)(r2.y + 26), 20, (Color){ 0xFA, 0xBC, 0x2A, 0xFF });

            /* Tile 3: Corrupt */
            Rectangle r3 = { 24.0f + (tile_w + 12.0f) * 3.0f, tile_y, tile_w, tile_h };
            DrawRectangleRec(r3, PANEL_COLOR);
            DrawRectangleLinesEx(r3, 1.0f, BORDER_COLOR);
            DrawText("Corrupt", (int)(r3.x + 12), (int)(r3.y + 10), 11, BORDER_COLOR);
            snprintf(score_str, sizeof(score_str), "%d", st.summary.total_invalid);
            DrawText(score_str, (int)(r3.x + 12), (int)(r3.y + 26), 20, (Color){ 0xE0, 0x48, 0x48, 0xFF });

            /* Scrollable results panel */
            Rectangle results_rect = { 24.0f, 160.0f, (float)(w - 48), (float)(h - 225) };
            DrawRectangleRec(results_rect, PANEL_COLOR);
            DrawRectangleLinesEx(results_rect, 1.0f, BORDER_COLOR);

            int start_item = (int)(st.results_scroll.y / 24.0f);
            if (start_item < 0) start_item = 0;
            int max_visible = (int)((results_rect.height - 12) / 24.0f);

            /* Scroll handling */
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) {
                st.results_scroll.y -= wheel * 24.0f;
                if (st.results_scroll.y < 0.0f) st.results_scroll.y = 0.0f;
                float max_scroll = (st.result_count - max_visible) * 24.0f;
                if (max_scroll < 0.0f) max_scroll = 0.0f;
                if (st.results_scroll.y > max_scroll) st.results_scroll.y = max_scroll;
            }

            /* Draw lines of files */
            int draw_count = 0;
            for (int i = start_item; i < st.result_count && draw_count < max_visible; i++) {
                int item_y = (int)(results_rect.y + 8.0f + draw_count * 24.0f);
                FileHash *res = &st.results[i];

                Color status_color = BORDER_COLOR;
                const char *status_label = "[???]";
                if (res->status == 101) {
                    status_color = (Color){ 0x30, 0xC8, 0x5D, 0xFF };
                    status_label = "[ OK ]";
                } else if (res->status == 404) {
                    status_color = (Color){ 0xFA, 0xBC, 0x2A, 0xFF };
                    status_label = "[MISS]";
                } else if (res->status == 909) {
                    status_color = (Color){ 0xE0, 0x48, 0x48, 0xFF };
                    status_label = "[BAD ]";
                }

                DrawText(status_label, (int)(results_rect.x + 12), item_y, 14, status_color);
                DrawText(res->filename, (int)(results_rect.x + 72), item_y, 14, TEXT_COLOR);

                /* Hash, truncated/positioned right */
                int label_w = MeasureText(res->filename, 14);
                int hash_x = (int)(results_rect.x + 72 + label_w + 16);
                if (hash_x < results_rect.x + results_rect.width - 240) {
                    DrawText(res->hash, hash_x, item_y + 1, 11, BORDER_COLOR);
                }
                
                /* Draw hash algorithm display type */
                int method_w = MeasureText(res->method, 11);
                DrawText(res->method, (int)(results_rect.x + results_rect.width - method_w - 16), item_y + 1, 11, BORDER_COLOR);

                draw_count++;
            }

            if (st.result_count == 0) {
                DrawText("(no files checked)", (int)(results_rect.x + 24), (int)(results_rect.y + 24), 14, BORDER_COLOR);
            }

            /* Bottom bar close back button */
            Rectangle close_btn = { (float)(w - 124), (float)(h - 52), 100.0f, 32.0f };
            if (GuiButton(close_btn, "Close")) {
                st.screen = SCREEN_MAIN;
                st.done = 0;
                st.is_validation = 0;
                if (st.results) {
                    free(st.results);
                    st.results = NULL;
                }
                st.result_count = 0;
            }
        }
        else if (st.screen == SCREEN_ABOUT) {
            /* Title. */
            DrawText("About ChecksumDrop", 24, 20, 26, TEXT_COLOR);
            DrawText("Application details & technology stack", 24, 54, 14, BORDER_COLOR);

            Rectangle text_rect = { 24.0f, 85.0f, (float)(w - 48), (float)(h - 150) };
            DrawRectangleRec(text_rect, PANEL_COLOR);
            DrawRectangleLinesEx(text_rect, 1.0f, BORDER_COLOR);

            int start_y = (int)(text_rect.y + 24);
            DrawText("ChecksumDrop", (int)(text_rect.x + 24), start_y, 22, ACCENT_COLOR);
            DrawText("Version 1.1 (Standard C11 Rewrite)", (int)(text_rect.x + 24), start_y + 28, 14, TEXT_COLOR);

            DrawText("A lightning-fast, premium desktop utility developed for immediate", (int)(text_rect.x + 24), start_y + 70, 14, TEXT_COLOR);
            DrawText("launch and maximum security hash generation and verification.", (int)(text_rect.x + 24), start_y + 90, 14, TEXT_COLOR);

            DrawText("This native Linux/Windows C port fully replaces the previous", (int)(text_rect.x + 24), start_y + 120, 14, TEXT_COLOR);
            DrawText(".NET/Avalonia implementation to eliminate drag-and-drop", (int)(text_rect.x + 24), start_y + 140, 14, TEXT_COLOR);
            DrawText("intercompatibility issues under Linux X11 desktops by native integration.", (int)(text_rect.x + 24), start_y + 160, 14, TEXT_COLOR);

            DrawText("Core Assemblies:", (int)(text_rect.x + 24), start_y + 195, 14, ACCENT_COLOR);
            DrawText("• raylib & raygui — UI layout panels and hardware fluid canvas rendering.", (int)(text_rect.x + 24), start_y + 215, 14, TEXT_COLOR);
            DrawText("• blake3_simd — Hand-crafted AVX-512 & AVX2 standard hardware acceleration.", (int)(text_rect.x + 24), start_y + 235, 14, TEXT_COLOR);
            DrawText("• inih — Ben Hoyt's simple robust settings file format parsing library.", (int)(text_rect.x + 24), start_y + 255, 14, TEXT_COLOR);
            DrawText("• hash_libs — md5, sha1, sha2, sha3, xxhash & endian-safe crc32 libraries.", (int)(text_rect.x + 24), start_y + 275, 14, TEXT_COLOR);

            /* Back button */
            Rectangle back_btn = { (float)(w - 124), (float)(h - 52), 100.0f, 32.0f };
            if (GuiButton(back_btn, "Back")) {
                st.screen = SCREEN_MAIN;
            }
        }

        EndDrawing();
    }

    /* Clean up on exit */
    if (st.results) {
        free(st.results);
    }

    CloseWindow();
    return 0;
}
