/*
 * main.c — ChecksumDrop (C / raylib + raygui port)
 *
 * Phase 0 scaffold: opens a dark themed window, registers the Linux XDND
 * proxy so drag-and-drop works under reparenting compositors (Cinnamon/
 * Mutter), and logs every dropped file/folder path to stdout and on screen.
 *
 * This proves the core feature Avalonia could not deliver on Linux/X11.
 * Hashing, generation, validation and the full UI arrive in later phases
 * (see ../docs/c-raygui-migration.md).
 */

#include "raylib.h"

/* RAYGUI_IMPLEMENTATION must be defined exactly once, here in main.c */
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#if defined(__linux__)
#include "x11_dnd.h"
#endif

#include <stdio.h>
#include <string.h>

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
    /* Echo to stdout for terminal/debug capture. */
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

int main(void)
{
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
        DrawText("C / raylib + raygui port  —  Phase 0 scaffold", 24, 54, 14,
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
