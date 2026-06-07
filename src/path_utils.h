#pragma once
#include <stddef.h>

/* Returns 1 if path exists and is a directory, 0 otherwise. */
int path_is_directory(const char *path);

/* Returns 1 if path exists and is a file, 0 otherwise. */
int path_is_file(const char *path);

/* Extracts the parent directory of `path`.
 * e.g., "/foo/bar/baz" -> "/foo/bar", "file.txt" -> ""
 * Returns the parent directory in `out_parent` (up to `max_size` bytes). */
void path_get_parent(const char *path, char *out_parent, size_t max_size);

/* Resolves input path to an absolute, normalized path.
 * Under Linux, resolves symlinks, relative segments, etc. via realpath.
 * Under Windows, uses _fullpath. */
void path_get_absolute(const char *path, char *out_absolute, size_t max_size);

/* Calculates the relative path of `file_path` with respect to `base_path`.
 * Standards-compliant:
 * - Emits forward slash '/' as path separator on all platforms.
 * - Trims any leading slashes.
 * e.g., base_path = "/home/tony/data", file_path = "/home/tony/data/sub/file.txt"
 * -> "sub/file.txt"
 */
void path_get_relative(const char *base_path, const char *file_path, char *out_relative, size_t max_size);

/* Combines two paths using the platform-specific directory separator.
 * Standardizes separators in the output (replaces multiple slashes). */
void path_combine(const char *p1, const char *p2, char *out_combined, size_t max_size);

/* Recursively lists all files in a directory and adds them to `out_files` array.
 * Caller is responsible for freeing the memory of the collected strings and the array itself.
 * Parameters:
 *   - `path`: Path to start listing from (directory or file).
 *   - `out_files`: Pointer to dynamically allocated list of string pointers.
 *   - `out_count`: Number of files collected.
 *   - `out_capacity`: Allocated capacity of list.
 */
int path_recursive_list(const char *path, char ***out_files, int *out_count, int *out_capacity);

/* Frees the list of strings returned by `path_recursive_list`. */
void path_free_list(char **files, int count);
