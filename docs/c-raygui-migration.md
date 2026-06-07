# ChecksumDrop — Migration to C + raygui

> **Status:** Design blueprint (no implementation yet)
> **Author of plan:** GitHub Copilot, for Tony Mattsson
> **Source app:** ChecksumDrop 1.1 (.NET 10 / Avalonia, cross-platform)
> **Target app:** ChecksumDrop (C11 / raylib 5.0 + raygui, cross-platform)
> **Reference implementation:** [`DragFlac`](../../DragFlac) — an existing C11/raygui app by the same author that already solves the hard parts (self-contained CMake build, cross-compilation, and **working Linux drag-and-drop**).

---

## 1. Why migrate, and why C + raygui

### 1.1 The concrete problem

ChecksumDrop is a drag-and-drop application — dropping files/folders onto the window is the primary interaction. **No released version of Avalonia implements external file drag-and-drop on Linux/X11.**

This was verified directly against the shipped assemblies:

- `Avalonia.X11.dll` for **both** 11.3.17 and 12.0.3 contains **zero** `Xdnd*` references (Avalonia's X11 atom field names appear as plain strings in the binary, so their absence is definitive).
- The app window never gets the `XdndAware` X11 property, so the OS never treats it as a drop target and no drag events ever fire.
- This is Avalonia issue [#6085](https://github.com/AvaloniaUI/Avalonia/issues/6085) (open for years). The fix is PR [#20926](https://github.com/AvaloniaUI/Avalonia/pull/20926), still **unmerged**, targeting the unreleased **12.1** milestone with a `wont-backport` label.

In short: the framework cannot deliver the app's core feature on the target platform, and there is no stable release date.

### 1.2 Why raylib/raygui is the answer

`DragFlac` is a working C11 + raylib/raygui app that drops files on the same machine (Garuda / Cinnamon / X11). raylib (via GLFW) **does** register `XdndAware` and exposes a clean drop API:

```c
if (IsFileDropped()) {
    FilePathList dropped = LoadDroppedFiles();
    /* ... process dropped.paths[0..dropped.count] ... */
    UnloadDroppedFiles(dropped);
}
```

It also already carries a small Linux workaround ([`DragFlac/src/x11_dnd.c`](../../DragFlac/src/x11_dnd.c)) for compositing/reparenting window managers (Cinnamon/Mutter), which we reuse verbatim.

### 1.3 Additional benefits

- **Self-contained build.** No system packages; everything is vendored or fetched by CMake and statically linked. Same single binary story DragFlac already ships.
- **Tiny footprint, no runtime.** No .NET runtime dependency.
- **Proven cross-compilation.** DragFlac already cross-compiles to Windows (mingw-w64 + NSIS installer) and macOS (osxcross universal binary).

### 1.4 Non-goals / known limitations

- **Wayland drag-and-drop is out of scope.** raylib's drop support is X11-based (GLFW). This matches the current Linux reality where the app runs under X11. Document this as a future item.
- No new hash algorithms beyond the existing 12.
- No localization.

---

## 2. What ChecksumDrop does (functional spec to preserve)

ChecksumDrop computes and verifies file checksums. The user drops files, folders, or existing
digest files onto the window; the app auto-detects intent:

| Dropped content | Mode | Result |
| --- | --- | --- |
| Files / folders | **Generation** | Recurse folders, hash every file, write one digest file next to the data |
| Existing digest file(s) (`.sfv`, `.md5.txt`, …) | **Validation** | Re-hash referenced files, compare, report pass/fail per file |

### 2.1 Supported algorithms (12)

| Enum | Display name | Digest token | Hex length | C source |
| --- | --- | --- | --- | --- |
| `CRC32` | CRC32 | `sfv` | 8 | implement directly (table) |
| `MD5` | MD5 | `md5` | 32 | vendored public-domain |
| `SHA1` | SHA1 | `sha1` | 40 | vendored public-domain |
| `SHA256` | SHA256 | `sha256` | 64 | vendored public-domain |
| `SHA384` | SHA384 | `sha384` | 96 | vendored public-domain |
| `SHA512` | SHA512 | `sha512` | 128 | vendored public-domain |
| `SHA3256` | SHA3-256 | `sha3-256` | 64 | vendored tiny_sha3 |
| `SHA3384` | SHA3-384 | `sha3-384` | 96 | vendored tiny_sha3 |
| `SHA3512` | SHA3-512 | `sha3-512` | 128 | vendored tiny_sha3 |
| `BLAKE3` | BLAKE3 | `blake3` | 64 | official BLAKE3 C lib |
| `XXHASH64` | xxHash64 | `xxh64` | 16 | vendored xxhash.h |
| `XXHASH3` | xxHash3 | `xxh3` | 16 | vendored xxhash.h |

Hashes are emitted as **UPPERCASE** hex.

### 2.2 End-to-end flow

```
USER DROPS files / folders / digest files
        │
        ▼
process(dropped_paths, method)
        │
        ├── recursively expand folders → flat file list
        ├── compute base_path = shortest common parent of all inputs
        ├── if ALL dropped paths are digest files ── VALIDATION
        └── else ─────────────────────────────────── GENERATION
        │
   GENERATION                          VALIDATION
   ├ hash each file                    ├ read digest file line-by-line
   ├ build output (relative paths)     ├ parse hash + filename per format
   ├ write checksum.{token}.txt/.sfv   ├ recompute + compare each file
   └ return output path                └ return per-file status + summary
        │
        ▼
   Results screen (validation) / status text (generation)
```

### 2.3 Output formats (must match byte-for-byte)

Header comment line (first line of every output file):

```
; Created with ChecksumDrop version 1.1 <date> using <token> hash
```

Body lines:

- **CRC32 (`.sfv`):** `relative/path  HASH`  (path first, hash last — SFV convention)
- **All others (`.txt`):** `HASH  relative/path`  (hash first — md5sum/shaXsum convention)

Output file naming (written into `base_path`):

- CRC32 → `checksum.sfv`
- Others → `checksum.{token}.txt` (e.g. `checksum.md5.txt`, `checksum.sha3-256.txt`)

Paths in the file are **relative to `base_path`**.

### 2.4 Validation details

- Parse each non-comment, non-empty line.
- **CRC32:** `filename<sep>hash`, hash is the last 8 hex chars.
- **Others:** `hash<sep>filename`, hash is the first *N* hex chars (per algorithm).
- Validate the hash token is hex (`[0-9a-fA-F]+`).
- Reconstruct path = `base_path` + normalized filename, recompute, compare.
- Per-file status codes (kept identical for UI mapping): `101` = OK/match, `404` = file not found, `909` = hash mismatch.
- Summary: totals for files / valid / not-found / invalid / digests, plus an overall pass/fail message (“All files are validated” vs “There were errors!”).

### 2.5 App metadata

```
name    = ChecksumDrop
author  = Tony Mattsson
company = u-x.cc
year    = 2024
version = 1.1
```

---

## 3. Reference architecture (what to copy from DragFlac)

DragFlac is a near-perfect structural template. The table maps DragFlac concepts to ChecksumDrop.

| DragFlac concept | DragFlac file | ChecksumDrop equivalent |
| --- | --- | --- |
| Window init + main loop + screen state machine | [`src/main.c`](../../DragFlac/src/main.c) | `src/main.c` |
| Drop target screen + recursive file collection + worker spawn | [`src/screen_main.c`](../../DragFlac/src/screen_main.c) | `src/screen_main.c` |
| Results table (scroll panel) | [`src/screen_results.c`](../../DragFlac/src/screen_results.c) | `src/screen_results.c` |
| Settings/About screen | [`src/screen_settings.c`](../../DragFlac/src/screen_settings.c) | `src/screen_about.c` |
| App state struct + atomics | [`src/ui_state.h`](../../DragFlac/src/ui_state.h) | `src/app_state.h` |
| INI settings load/save | [`src/settings.c`](../../DragFlac/src/settings.c) | `src/settings.c` |
| Per-result record | [`src/processed_file.h`](../../DragFlac/src/processed_file.h) | `src/file_hash.h` |
| Byte-size formatting | [`src/file_size.c`](../../DragFlac/src/file_size.c) | `src/file_size.c` (reuse) |
| Embedded TTF font | [`src/inter_font.h`](../../DragFlac/src/inter_font.h) | `src/app_font.h` |
| **Linux X11 DnD proxy** | [`src/x11_dnd.c`](../../DragFlac/src/x11_dnd.c) | `src/x11_dnd.c` (**copy as-is**) |
| Domain work (audio decode/encode) | `audio_reader.c`, `flac_encoder.c` | `hashing.c`, `processor.c`, `output_writer.c`, `digest_parser.c` |

What is **the same**: build system, drag-and-drop, main loop, threading model, settings approach,
UI widget vocabulary, cross-compile toolchains.

What is **new/different**: the domain layer is hashing instead of audio encoding, and we need an
extra **validation** path and a **results table** (DragFlac has the table pattern already).

---

## 4. Target project layout

```
ChecksumDropC/                      (new repo or folder; name TBD)
├── CMakeLists.txt                  # FetchContent + targets (modeled on DragFlac)
├── build.sh                        # linux | windows | osx | clean (copied + adapted)
├── cmake/
│   └── toolchain-mingw64.cmake     # copied from DragFlac
├── assets/
│   ├── logo.ico / logo.svg / *.png # app icon (from Resources/Icons)
│   └── ChecksumDrop.rc             # Windows PE icon embed
├── lib/
│   ├── raygui/raygui.h             # single-header GUI
│   ├── inih/ini.c + ini.h          # INI parser
│   ├── xxhash/xxhash.h             # xxHash64 + xxHash3 (header-only)
│   └── hash/                       # vendored MD5/SHA1/SHA2/SHA3 C sources
│       ├── md5.c/.h
│       ├── sha1.c/.h
│       ├── sha2.c/.h               # SHA-256/384/512
│       └── sha3.c/.h               # tiny_sha3 (Keccak)
└── src/
    ├── main.c                      # init, font, loop, screen router, x11 proxy call
    ├── app_state.h                 # AppState struct, screen enum, atomics
    ├── x11_dnd.c / .h              # Linux XdndProxy fix (from DragFlac)
    ├── app_font.h                  # embedded TTF byte array
    ├── checksum_method.h           # enum + metadata table (name/token/hexlen)
    ├── file_hash.h                 # per-file result record
    ├── hashing.c / .h              # compute_hash(path, method, out_hex)
    ├── crc32.c / .h                # CRC32 table implementation
    ├── processor.c / .h            # orchestration: expand, base path, mode, progress
    ├── output_writer.c / .h        # generate digest file text + naming
    ├── digest_parser.c / .h        # parse + validate digest files, summary
    ├── settings.c / .h             # INI persistence (selected method)
    ├── path_utils.c / .h           # recursive walk, common base, relative paths
    ├── file_size.c / .h            # human-readable sizes (from DragFlac)
    ├── screen_main.c / .h          # drop zone, method selector, progress
    ├── screen_results.c / .h       # validation results table
    └── screen_about.c / .h         # About box
```

---

## 5. Build system design

Mirror DragFlac's [`CMakeLists.txt`](../../DragFlac/CMakeLists.txt) and [`build.sh`](../../DragFlac/build.sh).

### 5.1 CMake essentials

```cmake
cmake_minimum_required(VERSION 3.20)
project(ChecksumDrop VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)   # allow raylib 5.0's older minimum under CMake 4.x
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

# raylib 5.0 — fetched, static
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0 GIT_SHALLOW TRUE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# raygui — single header (interface target)
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/raygui")

# inih — INI parser (settings)
add_library(inih STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/inih/ini.c")
target_include_directories(inih PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/inih")

# BLAKE3 — official C library, fetched
FetchContent_Declare(blake3
    GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
    GIT_TAG 1.5.4 GIT_SHALLOW TRUE
    SOURCE_SUBDIR c)
FetchContent_MakeAvailable(blake3)

# xxHash + MD5/SHA1/SHA2/SHA3 are header/source files under lib/, compiled into the app target.
```

The app target links `raylib raygui inih blake3` plus the vendored hash sources. Platform blocks
copied from DragFlac:

- **Linux:** `target_sources(... src/x11_dnd.c)`, include `${raylib_SOURCE_DIR}/src/external/glfw/include`, link `m pthread dl X11`.
- **Windows:** `WIN32_EXECUTABLE TRUE` (no console), link `winmm`, embed `assets/ChecksumDrop.rc`, CPack→NSIS installer into `$LOCALAPPDATA`.
- **macOS:** frameworks `Cocoa IOKit CoreAudio CoreVideo` (CoreAudio/Video are pulled by raylib; keep for parity).

### 5.2 build.sh

Copy DragFlac's script and rename the binary/product. Targets:

```bash
./build.sh linux     # native Debug → build/linux/ChecksumDrop
./build.sh windows   # mingw-w64 Release + NSIS installer → build/windows/ChecksumDrop.exe
./build.sh osx       # osxcross universal → build/osx/ChecksumDrop
./build.sh clean
```

Uses `-j$(nproc)`; Windows needs `x86_64-w64-mingw32-gcc` + `cmake/toolchain-mingw64.cmake`; macOS needs `OSXCROSS_ROOT`.

### 5.3 Dependency acquisition summary

| Need | C# library | C approach | How obtained |
| --- | --- | --- | --- |
| GUI + DnD | Avalonia | raylib 5.0 + raygui | FetchContent + `lib/raygui/raygui.h` |
| CRC32 | `Crc32Stream` (custom) | implement directly | `src/crc32.c` |
| MD5/SHA1/SHA2 | `System.Security.Cryptography` | public-domain C | `lib/hash/` (vendored) |
| SHA3 | `SHA3.Net` | tiny_sha3 (Keccak) | `lib/hash/sha3.*` (vendored) |
| BLAKE3 | `Blake3` | official BLAKE3 C lib | FetchContent (`SOURCE_SUBDIR c`) |
| xxHash64/3 | `System.IO.Hashing` | xxHash single header | `lib/xxhash/xxhash.h` (vendored) |
| Settings | `JsonSettingsService` (JSON) | **INI via inih** | `lib/inih/` (vendored) |

> **Recommendation applied:** settings move from JSON to **INI** (inih), matching DragFlac. The
> only persisted value is the selected method, so the format change is invisible to users.

---

## 6. Core data structures (C)

```c
/* checksum_method.h */
typedef enum {
    METHOD_CRC32, METHOD_MD5, METHOD_SHA1,
    METHOD_SHA256, METHOD_SHA384, METHOD_SHA512,
    METHOD_SHA3_256, METHOD_SHA3_384, METHOD_SHA3_512,
    METHOD_BLAKE3, METHOD_XXHASH64, METHOD_XXHASH3,
    METHOD_COUNT
} ChecksumMethod;

typedef struct {
    const char *display_name;   /* "SHA3-256"            */
    const char *token;          /* "sha3-256"            */
    int         hex_length;     /* 64                    */
} MethodInfo;

/* Single source of truth, indexed by ChecksumMethod */
extern const MethodInfo METHOD_TABLE[METHOD_COUNT];

/* file_hash.h */
typedef struct {
    char filename[1024];   /* relative path                          */
    char hash[129];        /* uppercase hex, max 128 + NUL           */
    int  status;           /* 101 ok / 404 not found / 909 mismatch  */
} FileHash;

/* validation summary */
typedef struct {
    int total_files, total_valid, total_not_found, total_invalid, total_digests;
} ValidationSummary;
```

`app_state.h` (modeled on DragFlac's `ui_state.h`):

```c
typedef enum { SCREEN_MAIN, SCREEN_RESULTS, SCREEN_ABOUT } AppScreen;

typedef struct {
    AppScreen      screen;
    ChecksumMethod method;          /* persisted */

    /* worker ↔ UI communication (no mutex needed) */
    _Atomic float  progress;        /* 0.0 – 1.0     */
    _Atomic int    busy;            /* operation running */
    _Atomic int    done;            /* operation finished */
    _Atomic int    cancel_requested;

    /* status strings (main + sub lines, mirroring ChecksumProgress) */
    char main_text[256], sub_text[256], sub_sub_text[256], status_text[256];

    /* results (written by worker, read by UI after `done`) */
    FileHash         *results;      /* dynamic array */
    int               result_count;
    int               is_validation;
    ValidationSummary summary;
    char              output_path[1024];

    /* scroll state for results table */
    Vector2 results_scroll;
} AppState;
```

---

## 7. Hashing module

A single streaming entry point keeps callers simple:

```c
/* hashing.h
 * Computes `method` over file at `path`, writes UPPERCASE hex into out_hex
 * (must hold METHOD_TABLE[method].hex_length + 1). Returns 0 on success. */
int compute_hash(const char *path, ChecksumMethod method, char *out_hex);
```

Implementation notes (preserve exact behavior from `HashingService.cs`):

- **Streaming with explicit buffers.** CRC32 uses a **4096**-byte buffer; BLAKE3 and xxHash use **16384**. SHA1/MD5/SHA2/SHA3 stream in a 16384 buffer for consistency.
- **CRC32** constants: polynomial `0xEDB88320`, init `0xFFFFFFFF`, final XOR `0xFFFFFFFF`, 256-entry table; output 8-char uppercase hex.
- **SHA3** must handle **0-byte files** correctly (the C# code special-cased empty input via SHA3.Net — tiny_sha3 handles empty input natively; add a test).
- **BLAKE3**: `blake3_hasher_init` → `blake3_hasher_update` in chunks → `blake3_hasher_finalize` to 32 bytes → hex.
- **xxHash**: define `XXH_INLINE_ALL` once; use `XXH64`/`XXH3_64bits` streaming state (`XXH64_update` / `XXH3_64bits_update`), then format the 64-bit digest as 16-char uppercase hex.
- All outputs uppercase; a shared `to_hex_upper(bytes, n, out)` helper.

Per-algorithm dispatch in `compute_hash` is a `switch (method)`.

---

## 8. Processor module

Port of `ChecksumProcessor.ProcessAsync` plus `path_utils`.

```c
/* processor.h */
typedef void (*ProgressCb)(AppState *st, float pct,
                           const char *main, const char *sub, const char *status);

/* Runs on the worker thread. Fills st->results / summary / output_path. */
void processor_run(AppState *st, char **dropped_paths, int dropped_count);
```

Responsibilities:

1. **Expand inputs.** For each dropped path: if a directory, recurse (POSIX `opendir`/`readdir`, Windows `FindFirstFile`/`FindNextFile`) collecting all files; if a file, add directly. (`path_utils.c`.)
2. **Compute base path** = shortest common parent directory of all inputs. Used to make output paths relative.
3. **Mode auto-detect.** If **every** dropped path parses as a digest file (filename contains a known token / matches naming), run **validation**; otherwise **generation**.
4. **Generation:** `compute_hash` per file → `output_writer_generate` → write file → set `output_path`.
5. **Validation:** `digest_parser` per digest file → recompute/compare → fill `results` + `summary`.
6. **Progress:** invoke callback / update atomics after each file. Use a line-count pre-pass (port of `LineCounter`, 1 MB chunks, CR/LF/CRLF aware) for validation percentage.
7. **Cancellation:** check `st->cancel_requested` between files.

Directory recursion and path handling are the main platform-specific surface; isolate them in
`path_utils.c` behind `#if defined(_WIN32)`.

---

## 9. Output writer & digest parser

### 9.1 output_writer.c

```c
/* Builds the full digest file text for generation mode and returns the
 * output file name (checksum.sfv or checksum.<token>.txt). */
char *output_writer_generate(const FileHash *hashes, int count,
                             ChecksumMethod method, const char *base_path,
                             char *out_filename, size_t out_filename_size);
```

- First line: `; Created with ChecksumDrop version 1.1 <date> using <token> hash`.
- CRC32 line order: `relpath  HASH`; all others: `HASH  relpath`.
- Two-space separator (match current output; verify against fixtures in [`Resources/hashes`](../Resources/hashes)).
- Relative paths use `/` separators in the file regardless of OS (verify against current behavior).

### 9.2 digest_parser.c

```c
int digest_parser_is_digest_file(const char *path);   /* for mode detection */

int digest_parser_validate(const char *digest_path, ChecksumMethod method,
                           const char *base_path,
                           FileHash **out_results, int *out_count,
                           ValidationSummary *out_summary,
                           AppState *st /* for progress + cancel */);
```

- Skip comment lines (leading `;`) and blank lines.
- CRC32: hash = last 8 hex chars of the line; filename = remainder. Others: hash = first *N* hex; filename = remainder.
- Hex-validate the token (`[0-9a-fA-F]+`); reject malformed lines.
- Reconstruct path = `base_path` + normalized filename; recompute with `compute_hash`; compare case-insensitively.
- Set status `101` / `404` / `909`; accumulate `ValidationSummary`.

---

## 10. Settings module

Port of `JsonSettingsService` to **INI** via inih (recommended), modeled on DragFlac's `settings.c`.

- File location:
  - Linux: `$XDG_CONFIG_HOME/ChecksumDrop/settings.ini` (fallback `~/.config/ChecksumDrop/`)
  - Windows: `%APPDATA%\ChecksumDrop\settings.ini`
  - macOS: `~/Library/Application Support/ChecksumDrop/settings.ini`
- Content:

  ```ini
  [general]
  method = md5
  ```

- `settings_load(AppState*)` at startup; `settings_save(AppState*)` on method change and at exit.
- Persisted value is the method **token** (stable, human-readable). Map token → `ChecksumMethod` via `METHOD_TABLE`.

---

## 11. UI design (raygui)

> **Recommendation applied:** replicate ChecksumDrop's existing **dark** theme (background ~`#10161F`, panel `#17212B`, border `#3A4758`, white text) via `GuiSetStyle`, rather than DragFlac's grey. Use an embedded TTF for crisp text (DragFlac embeds Inter-Regular; reuse it).

### 11.1 Screen: Main (drop + generate)

Mirrors `MainWindow.axaml` / `MainWindowViewModel`.

| Element | raygui widget | Notes |
| --- | --- | --- |
| Drop zone panel | `GuiPanel` + bordered `DrawRectangleRec` | "Drop files or folders anywhere in this panel" |
| Big status lines | `DrawTextEx` with app font | `main_text` / `sub_text` / `sub_sub_text` |
| Method selector | `GuiComboBox` (or `GuiDropdownBox`) | items = `METHOD_TABLE[i].display_name`; selecting persists settings |
| Choose Files | `GuiButton` | reliable fallback to DnD (uses native file dialog or path entry) |
| Choose Folder | `GuiButton` | same |
| About | `GuiButton` | switches `screen = SCREEN_ABOUT` |
| Progress | `GuiProgressBar` | bound to `st->progress`; visible while `busy` |
| Status text | `GuiLabel` / `DrawText` | `status_text` |

> **File/Folder pickers:** raylib has no native file dialog. Options: (a) integrate
> [`tinyfiledialogs`](https://sourceforge.net/projects/tinyfiledialogs/) (single C file, vendor in `lib/`),
> or (b) since drag-and-drop now works, keep the buttons but back them with tinyfiledialogs.
> **Recommendation:** vendor `tinyfiledialogs` for parity with the current Choose Files/Folder buttons.

### 11.2 Screen: Results (validation)

Port of `ResultsWindow` using DragFlac's scroll-table pattern ([`screen_results.c`](../../DragFlac/src/screen_results.c)):

- `GuiScrollPanel` + `BeginScissorMode`/`EndScissorMode`, cull rows outside the viewport.
- Columns: Filename, Method, Status.
- Status color: green for `101`, red for `404`/`909`. Status text: "OK" / "File not found" / "Invalid hash".
- Summary footer: overall pass/fail + totals (files / not found / invalid).
- Alternating row backgrounds (DragFlac already does this).

### 11.3 Screen: About

Simple panel (or `GuiMessageBox`) showing name/version/author/company/year from app metadata and a Close button. Matches the recent Settings→About change in the Avalonia app.

### 11.4 Main loop (src/main.c)

```c
InitWindow(WINDOW_W, WINDOW_H, "ChecksumDrop");
SetTargetFPS(60);
SetWindowState(FLAG_WINDOW_RESIZABLE);
SetWindowMinSize(560, 560);

#if defined(__linux__)
    xdnd_proxy_setup(GetWindowHandle());   /* register XdndProxy on WM frame */
#endif

Font app_font = LoadFontFromMemory(".ttf", app_font_ttf, app_font_ttf_len, 20, NULL, 0);
GuiSetFont(app_font);
apply_dark_theme();                        /* GuiSetStyle(...) dark palette */

settings_load(&state);

while (!WindowShouldClose()) {
    handle_drop(&state);                   /* IsFileDropped → spawn worker */
    poll_worker(&state);                   /* atomics → transition to results */

    BeginDrawing();
    ClearBackground(BG_COLOR);
    switch (state.screen) {
        case SCREEN_MAIN:    screen_main_draw(&state, app_font);    break;
        case SCREEN_RESULTS: screen_results_draw(&state, app_font); break;
        case SCREEN_ABOUT:   screen_about_draw(&state, app_font);   break;
    }
    EndDrawing();
}

settings_save(&state);
UnloadFont(app_font);
CloseWindow();
```

---

## 12. Drag-and-drop (the whole point)

1. **Detection (all platforms):** in `handle_drop`, call `IsFileDropped()`; on true, `LoadDroppedFiles()`, copy `paths[0..count]`, `UnloadDroppedFiles()`, then start the worker.
2. **Linux fix:** copy [`DragFlac/src/x11_dnd.c`](../../DragFlac/src/x11_dnd.c) and its header **verbatim**. Call `xdnd_proxy_setup(GetWindowHandle())` once, immediately after `InitWindow`. This sets `XdndAware` + `XdndProxy` on the WM frame window so reparenting compositors (Cinnamon/Mutter) forward `XdndEnter/Position/Drop` to the real app window.
3. **Verification:** after launch, `xprop -id <app-window> | grep -i xdnd` must show `XdndAware`. (Find the window by `WM_CLASS` = the binary name; do **not** match by title — that previously gave a false positive against the VS Code window.)

This is exactly the gap Avalonia could not fill, and the reason the migration exists.

---

## 13. Threading model

Identical to DragFlac:

- Worker thread via POSIX `pthread_create` + `pthread_detach` (mingw provides pthreads; or use a thin wrapper / C11 `<threads.h>` if preferred).
- Communication via `_Atomic` fields: `progress`, `busy`, `done`, `cancel_requested`. No mutex required — the results array is written only by the worker and read by the UI **after** `done` flips.
- UI polls atomics each frame (`poll_worker`) and, on `done`, transitions to the results screen (validation) or shows the output path (generation).
- Cancel button sets `cancel_requested`; the worker checks it between files.

---

## 14. Cross-platform notes

- **Paths:** isolate separators and recursion in `path_utils.c` (`#if defined(_WIN32)`); output files always use `/`.
- **Config dir:** per §10.
- **Icon:** reuse [`Resources/Icons`](../Resources/Icons); export a multi-size `.ico`, embed via `assets/ChecksumDrop.rc` (Windows) and `SetWindowIcon` from an embedded image (Linux/macOS, optional).
- **Font:** embed a TTF as a byte array (`app_font.h`), like DragFlac's `inter_font.h`.
- **No console window on Windows:** `WIN32_EXECUTABLE TRUE`.

---

## 15. Phased implementation roadmap

Each phase ends with a concrete verification, several against the existing fixtures in
[`Resources/hashes`](../Resources/hashes) and the current Avalonia app's output.

| Phase | Deliverable | Verification |
| --- | --- | --- |
| **0 — Scaffold** | CMake + raylib/raygui + empty dark window + `x11_dnd.c` + drop logging | Window opens on Linux/Win/macOS; `xprop` shows `XdndAware`; a dropped file path is logged on all three OSes |
| **1 — Hashing** | `hashing.c` + `crc32.c` + vendored libs; CLI/test harness | Hash known inputs; compare every algorithm's output against the current app and against `Resources/hashes` samples |
| **2 — Generation** | `processor.c` (generation) + `output_writer.c` + `path_utils.c` | Drop the same folder into both apps; **byte-diff** the generated `checksum.*` files (header date excepted) |
| **3 — Validation** | `digest_parser.c` + validation flow + summary | Validate existing `.sfv`/`.md5`/`.sha1`/hashdeep fixtures; statuses and totals match the current app |
| **4 — UI + settings** | Main/Results/About screens, method selector, `settings.c` (INI), file/folder pickers | Full interactive flow; method persists across restart; results table matches validation data |
| **5 — Threading + packaging** | Worker thread, progress, cancel; Windows NSIS + macOS universal builds | Large folder hashes with live progress and working cancel; `build.sh windows`/`osx` produce installers/binaries |

**Suggested ground-truth command** for phases 1–3 (compare against current app output):

```bash
# generate with current app, then with the C port, into separate dirs, then:
diff <(tail -n +2 checksum.md5.txt) <(tail -n +2 ../c-out/checksum.md5.txt)
# (skip line 1: the header carries a date)
```

---

## 16. Open questions

These are resolved with recommendations baked into this document; flag if any should change:

1. **Settings format** — moved to **INI** (inih) for consistency with DragFlac. *(Recommended; reversible.)*
2. **UI theme** — **replicate ChecksumDrop's dark palette** rather than DragFlac's grey. *(Recommended.)*
3. **About vs Settings screen** — **simple About box** (no settings screen; the method selector lives on the main screen). *(Recommended; matches current app.)*
4. **Build tooling** — **copy DragFlac's `build.sh`, `cmake/toolchain-mingw64.cmake`, and CPack/NSIS setup** and adapt names. *(Recommended.)*
5. **File/folder pickers** — vendor **tinyfiledialogs** to back the Choose Files/Folder buttons (raylib has no native dialog). *(Recommended; confirm before Phase 4.)*
6. **Repository** — new top-level folder/repo (e.g. `ChecksumDropC/`) vs. a subfolder of this repo. *(Decision needed before Phase 0.)*

---

## 17. Risk register

| Risk | Mitigation |
| --- | --- |
| Output not byte-identical to current app | Phase-2/3 diff gates against `Resources/hashes` fixtures and live output |
| Hash mismatch on edge cases (empty files, large files) | Dedicated tests; SHA3/BLAKE3 empty-input cases called out in §7 |
| Wayland sessions (no X11 DnD) | Documented non-goal; buttons + tinyfiledialogs remain functional |
| Reparenting WM still blocks drops | `x11_dnd.c` proxy already handles Cinnamon/Mutter; verify with `xprop` |
| Cross-compile breakage (mingw/osxcross) | Reuse DragFlac's proven toolchains unchanged; vendored deps avoid system-package drift |
| BLAKE3 build via FetchContent on all targets | If problematic, vendor BLAKE3's portable C sources directly under `lib/hash/` |

---

*End of design document.*
