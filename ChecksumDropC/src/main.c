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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Window & palette (ChecksumDrop dark theme) ──────────────────────────── */
#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 560

#define BG_COLOR      (Color){ 0x10, 0x16, 0x1F, 0xFF }  /* #10161F window     */
#define PANEL_COLOR   (Color){ 0x17, 0x21, 0x2B, 0xFF }  /* #17212B drop zone  */
#define BORDER_COLOR  (Color){ 0x3A, 0x47, 0x58, 0xFF }  /* #3A4758 border     */
#define TEXT_COLOR    (Color){ 0xEA, 0xEF, 0xF5, 0xFF }  /* near-white text    */
#define ACCENT_COLOR  (Color){ 0x4C, 0x8B, 0xF5, 0xFF }  /* accent blue        */

/* Simple ring buffer of the most recent dropped paths (Phase 0 diagnostics). */
#define MAX_LOG_LINES 16
#define MAX_PATH_LEN  1024

static char  s_log[MAX_LOG_LINES][MAX_PATH_LEN];
static int   s_log_count = 0;

static void log_push(const char *path)
{
    printf("[drop] %s\n", path);
    fflush(stdout);

    if (s_log_count < MAX_LOG_LINES) {
        snprintf(s_log[s_log_count], MAX_PATH_LEN, "%s", path);
        s_log_count++;
    } else {
        /* Scroll up: drop the oldest line. */
        for (int i = 1; i < MAX_LOG_LINES; i++) {
            memcpy(s_log[i - 1], s_log[i], MAX_PATH_LEN);
        }
        snprintf(s_log[MAX_LOG_LINES - 1], MAX_PATH_LEN, "%s", path);
    }
}

static void handle_drop(void)
{
    if (!IsFileDropped()) return;

    FilePathList dropped = LoadDroppedFiles();
    for (unsigned int i = 0; i < dropped.count; i++) {
        log_push(dropped.paths[i]);
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

    while (!WindowShouldClose()) {
        handle_drop();

        BeginDrawing();
        ClearBackground(BG_COLOR);

        int w = GetScreenWidth();
        int h = GetScreenHeight();

        /* Title. */
        DrawText("ChecksumDrop", 24, 20, 26, TEXT_COLOR);
        DrawText("C / raylib + raygui port  —  Phase 1 Hashing", 24, 54, 14,
                 BORDER_COLOR);

        /* Drop zone panel. */
        const int pad = 24;
        const int zone_y = 90;
        const int zone_h = 160;
        Rectangle zone = { (float)pad, (float)zone_y,
                           (float)(w - pad * 2), (float)zone_h };
        DrawRectangleRec(zone, PANEL_COLOR);
        DrawRectangleLinesEx(zone, 2.0f, BORDER_COLOR);

        const char *prompt = "Drop files or folders anywhere in this panel";
        int tw = MeasureText(prompt, 20);
        DrawText(prompt, (int)(zone.x + (zone.width - tw) / 2),
                 (int)(zone.y + zone_h / 2 - 10), 20, TEXT_COLOR);

        /* Dropped-path log. */
        DrawText("Dropped paths:", pad, zone_y + zone_h + 20, 16, ACCENT_COLOR);
        int line_y = zone_y + zone_h + 46;
        if (s_log_count == 0) {
            DrawText("(none yet — drag something onto the window)",
                     pad, line_y, 14, BORDER_COLOR);
        } else {
            for (int i = 0; i < s_log_count && line_y < h - 20; i++) {
                DrawText(s_log[i], pad, line_y, 14, TEXT_COLOR);
                line_y += 20;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
