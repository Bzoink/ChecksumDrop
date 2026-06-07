#include "path_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#endif

int path_is_directory(const char *path) {
    if (!path || strlen(path) == 0) return 0;
#if defined(_WIN32)
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
#endif
}

int path_is_file(const char *path) {
    if (!path || strlen(path) == 0) return 0;
#if defined(_WIN32)
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
#endif
}

void path_get_parent(const char *path, char *out_parent, size_t max_size) {
    if (!path || !out_parent || max_size == 0) return;

    size_t len = strlen(path);
    if (len == 0) {
        out_parent[0] = '\0';
        return;
    }

    /* Copy path and strip trailing slashes (except root) */
    char temp[4096];
    if (len >= sizeof(temp)) len = sizeof(temp) - 1;
    memcpy(temp, path, len);
    temp[len] = '\0';

    while (len > 1 && (temp[len - 1] == '/' || temp[len - 1] == '\\')) {
        temp[len - 1] = '\0';
        len--;
    }

    /* Find last separator */
    char *last_slash = strrchr(temp, '/');
    char *last_back = strrchr(temp, '\\');
    char *separator = last_slash;
    if (last_back && (!separator || last_back > separator)) {
        separator = last_back;
    }

    if (separator) {
        if (separator == temp) {
            /* Root folder (e.g., "/" or "\") */
            snprintf(out_parent, max_size, "%c", separator[0]);
        } else {
            /* Truncate at separator */
            size_t parent_len = separator - temp;
            if (parent_len >= max_size) parent_len = max_size - 1;
            memcpy(out_parent, temp, parent_len);
            out_parent[parent_len] = '\0';
        }
    } else {
        /* No parent separator (e.g. raw filename in local directory) */
        out_parent[0] = '\0';
    }
}

void path_get_absolute(const char *path, char *out_absolute, size_t max_size) {
    if (!path || !out_absolute || max_size == 0) return;

#if defined(_WIN32)
    char *res = _fullpath(out_absolute, path, max_size);
    if (!res) {
        snprintf(out_absolute, max_size, "%s", path);
    }
#else
    char resolved[PATH_MAX];
    char *res = realpath(path, resolved);
    if (res) {
        snprintf(out_absolute, max_size, "%s", resolved);
    } else {
        /* Fallback if realpath fails (e.g., file doesn't exist yet) */
        if (path[0] == '/') {
            snprintf(out_absolute, max_size, "%s", path);
        } else {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd))) {
                path_combine(cwd, path, out_absolute, max_size);
            } else {
                snprintf(out_absolute, max_size, "%s", path);
            }
        }
    }
#endif
}

void path_get_relative(const char *base_path, const char *file_path, char *out_relative, size_t max_size) {
    if (!file_path || !out_relative || max_size == 0) return;

    if (!base_path || strlen(base_path) == 0) {
        /* No base path -> return file_path with forward slashes */
        snprintf(out_relative, max_size, "%s", file_path);
        for (size_t i = 0; out_relative[i]; i++) {
            if (out_relative[i] == '\\') out_relative[i] = '/';
        }
        return;
    }

    char abs_base[4096];
    char abs_file[4096];
    path_get_absolute(base_path, abs_base, sizeof(abs_base));
    path_get_absolute(file_path, abs_file, sizeof(abs_file));

    /* Replace backslashes with forward slashes for unified comparison */
    for (size_t i = 0; abs_base[i]; i++) {
        if (abs_base[i] == '\\') abs_base[i] = '/';
    }
    for (size_t i = 0; abs_file[i]; i++) {
        if (abs_file[i] == '\\') abs_file[i] = '/';
    }

    size_t base_len = strlen(abs_base);
    size_t file_len = strlen(abs_file);

    /* Strip trailing slashes on base */
    while (base_len > 1 && abs_base[base_len - 1] == '/') {
        abs_base[base_len - 1] = '\0';
        base_len--;
    }

#if defined(_WIN32)
    /* Case-insensitive base path match under Windows */
    int prefix_match = (_strnicmp(abs_file, abs_base, base_len) == 0);
#else
    int prefix_match = (strncmp(abs_file, abs_base, base_len) == 0);
#endif

    if (prefix_match) {
        const char *rel = abs_file + base_len;
        while (*rel == '/') {
            rel++;
        }
        snprintf(out_relative, max_size, "%s", rel);
    } else {
        /* Fallback: copy file path directly and normalize */
        snprintf(out_relative, max_size, "%s", abs_file);
    }

    /* Standardize all output separators to / */
    for (size_t i = 0; out_relative[i]; i++) {
        if (out_relative[i] == '\\') out_relative[i] = '/';
    }
}

void path_combine(const char *p1, const char *p2, char *out_combined, size_t max_size) {
    if (!out_combined || max_size == 0) return;

    if (!p1 || strlen(p1) == 0) {
        if (p2) snprintf(out_combined, max_size, "%s", p2);
        else out_combined[0] = '\0';
        return;
    }
    if (!p2 || strlen(p2) == 0) {
        snprintf(out_combined, max_size, "%s", p1);
        return;
    }

    char sep = '/';
#if defined(_WIN32)
    sep = '\\';
#endif

    size_t len1 = strlen(p1);
    int has_sep1 = (p1[len1 - 1] == '/' || p1[len1 - 1] == '\\');
    int has_sep2 = (p2[0] == '/' || p2[0] == '\\');

    if (has_sep1 && has_sep2) {
        snprintf(out_combined, max_size, "%s%s", p1, p2 + 1);
    } else if (has_sep1 || has_sep2) {
        snprintf(out_combined, max_size, "%s%s", p1, p2);
    } else {
        snprintf(out_combined, max_size, "%s%c%s", p1, sep, p2);
    }
}

static int path_list_internal(const char *current_path, char ***out_files, int *out_count, int *out_capacity) {
    if (path_is_file(current_path)) {
        if (*out_count >= *out_capacity) {
            *out_capacity = (*out_capacity == 0) ? 128 : (*out_capacity * 2);
            char **new_files = realloc(*out_files, *out_capacity * sizeof(char *));
            if (!new_files) return 0;
            *out_files = new_files;
        }
        (*out_files)[*out_count] = strdup(current_path);
        if (!(*out_files)[*out_count]) return 0;
        (*out_count)++;
        return 1;
    }

    if (!path_is_directory(current_path)) {
        return 1; /* Skip invalid trails, but don't fail overall */
    }

#if defined(_WIN32)
    char search_pattern[4096];
    path_combine(current_path, "*", search_pattern, sizeof(search_pattern));

    WIN32_FIND_DATAA find_data;
    HANDLE find_handle = FindFirstFileA(search_pattern, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) return 1;

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        char item_path[4096];
        path_combine(current_path, find_data.cFileName, item_path, sizeof(item_path));

        if (!path_list_internal(item_path, out_files, out_count, out_capacity)) {
            FindClose(find_handle);
            return 0;
        }
    } while (FindNextFileA(find_handle, &find_data));

    FindClose(find_handle);
#else
    DIR *dir = opendir(current_path);
    if (!dir) return 1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char item_path[4096];
        path_combine(current_path, entry->d_name, item_path, sizeof(item_path));

        if (!path_list_internal(item_path, out_files, out_count, out_capacity)) {
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
#endif

    return 1;
}

int path_recursive_list(const char *path, char ***out_files, int *out_count, int *out_capacity) {
    if (!path || !out_files || !out_count || !out_capacity) return 0;
    return path_list_internal(path, out_files, out_count, out_capacity);
}

void path_free_list(char **files, int count) {
    if (!files) return;
    for (int i = 0; i < count; i++) {
        if (files[i]) free(files[i]);
    }
    free(files);
}
