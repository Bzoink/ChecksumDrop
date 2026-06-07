#pragma once

#include "raylib.h"
#include "checksum_method.h"
#include "file_hash.h"

typedef enum {
    SCREEN_MAIN,
    SCREEN_RESULTS,
    SCREEN_ABOUT
} AppScreen;

typedef struct {
    int total_files;
    int total_valid;
    int total_not_found;
    int total_invalid;
    int total_digests;
} ValidationSummary;

typedef struct {
    AppScreen screen;
    ChecksumMethod method;              /* persisted settings */

    /* worker <-> UI dual-thread communication (Phase 5 will use threads, Phase 2 is synchronous) */
    volatile float progress;            /* 0.0 - 1.0 */
    volatile int   busy;                /* is operation running */
    volatile int   done;                /* is operation finished successfully */
    volatile int   cancel_requested;

    /* status lines (sub and sub-sub lines matching ChecksumProgress) */
    char main_text[256];
    char sub_text[256];
    char sub_sub_text[256];
    char status_text[256];

    /* results (dynamically allocated array of file hash items, read by UI after `done`) */
    FileHash *results;
    int       result_count;
    int       is_validation;
    ValidationSummary summary;
    char      output_path[1024];

    /* Scroll position for results table */
    Vector2 results_scroll;
} AppState;
