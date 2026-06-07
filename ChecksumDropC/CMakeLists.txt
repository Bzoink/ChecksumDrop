cmake_minimum_required(VERSION 3.20)
project(ChecksumDrop VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Allow raylib 5.0's older cmake_minimum_required under CMake 4.x
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Static linking for all fetched libraries — no system packages required
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ── raylib ────────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.0
    GIT_SHALLOW    TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# ── raygui (single header, lib/raygui/) ───────────────────────────────────────
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/raygui"
)

# ── inih (two-file INI parser, lib/inih/) ─────────────────────────────────────
add_library(inih STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/inih/ini.c"
)
target_include_directories(inih PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/inih"
)

# ── BLAKE3 — official C library, fetched ──────────────────────────────────────
FetchContent_Declare(blake3
    GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
    GIT_TAG        1.5.4
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  c
)
FetchContent_MakeAvailable(blake3)

# ── ChecksumDrop executable ───────────────────────────────────────────────────
set(SOURCES
    src/main.c
    src/checksum_method.c
    src/crc32.c
    src/hashing.c
    src/path_utils.c
    src/output_writer.c
    src/processor.c
    src/settings.c
    lib/hash/md5.c
    lib/hash/sha1.c
    lib/hash/sha2.c
    lib/hash/sha3.c
)

# Embed the application icon into the Windows PE executable.
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/ChecksumDrop.rc")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/ChecksumDrop.rc"
        "${CMAKE_CURRENT_BINARY_DIR}/ChecksumDrop.rc"
        COPYONLY
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.ico"
        "${CMAKE_CURRENT_BINARY_DIR}/logo.ico"
        COPYONLY
    )
    list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/ChecksumDrop.rc")
endif()

add_executable(ChecksumDrop ${SOURCES})

target_include_directories(ChecksumDrop PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/hash"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/xxhash"
)

if(WIN32)
    # Use the Windows GUI subsystem — suppresses the console window.
    set_target_properties(ChecksumDrop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(ChecksumDrop PRIVATE
    raylib
    raygui
    inih
    blake3
)

if(WIN32)
    target_link_libraries(ChecksumDrop PRIVATE winmm)
endif()
if(APPLE)
    target_link_libraries(ChecksumDrop PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreAudio"
        "-framework CoreVideo"
    )
endif()
if(UNIX AND NOT APPLE)
    target_sources(ChecksumDrop PRIVATE src/x11_dnd.c)
    target_include_directories(ChecksumDrop PRIVATE
        "${raylib_SOURCE_DIR}/src/external/glfw/include"
    )
    target_link_libraries(ChecksumDrop PRIVATE m pthread dl X11)
endif()

# ── install rules ─────────────────────────────────────────────────────────────
install(TARGETS ChecksumDrop
    RUNTIME DESTINATION .
)

# ── packaging (CPack) ─────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME "ChecksumDrop")
set(CPACK_PACKAGE_VENDOR "u-x.cc")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Drag and drop files to compute and verify checksums")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ChecksumDrop")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "ChecksumDrop")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")

    # Default to per-user install location (local account) instead of Program Files.
    set(CPACK_NSIS_INSTALL_ROOT "$LOCALAPPDATA")
    set(CPACK_NSIS_PACKAGE_NAME "ChecksumDrop")
    set(CPACK_NSIS_DISPLAY_NAME "ChecksumDrop")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.ico")
        set(CPACK_NSIS_MUI_ICON
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.ico")
        set(CPACK_NSIS_MUI_UNIICON
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.ico")
    endif()
endif()

include(CPack)
