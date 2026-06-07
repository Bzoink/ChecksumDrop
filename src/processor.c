#include "processor.h"
#include "path_utils.h"
#include "hashing.h"
#include "output_writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int is_hex_string(const char *str) {
    if (!str || *str == '\0') return 0;
    while (*str) {
        char c = *str;
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return 0;
        }
        str++;
    }
    return 1;
}

static int count_file_lines(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    int count = 0;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        count++;
    }
    fclose(f);
    return count;
}

static void strip_newline(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
}

static void trim_string(char *str) {
    if (!str) return;
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t') {
        start++;
    }
    if (start > 0) {
        memmove(str, str + start, len - start + 1);
    }
}

static void normalize_digest_path(const char *raw_rel_path, char *out_path, size_t max_size) {
    if (!raw_rel_path || max_size == 0) return;
    
    char sep = '/';
#if defined(_WIN32)
    sep = '\\';
#endif

    size_t i = 0;
    size_t out_idx = 0;
    
    /* Skip leading separators */
    while (raw_rel_path[i] == '/' || raw_rel_path[i] == '\\') {
        i++;
    }
    
    for (; raw_rel_path[i] && out_idx < max_size - 1; i++) {
        char c = raw_rel_path[i];
        if (c == '/' || c == '\\') {
            out_path[out_idx++] = sep;
        } else {
            out_path[out_idx++] = c;
        }
    }
    out_path[out_idx] = '\0';
}

static void string_to_upper(char *str) {
    for (size_t i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'f') {
            str[i] = str[i] - 'a' + 'A';
        }
    }
}

static void run_create_digests(AppState *st, char **file_collection, int file_count, const char *base_path) {
    st->is_validation = 0;
    st->progress = 0.0f;
    st->busy = 1;
    st->done = 0;

    int total_items = file_count > 0 ? file_count : 1;

    /* Allocate memory for results */
    st->results = calloc(file_count, sizeof(FileHash));
    if (!st->results) {
        snprintf(st->status_text, sizeof(st->status_text), "Memory allocation failed!");
        st->busy = 0;
        return;
    }
    st->result_count = file_count;

    for (int i = 0; i < file_count; i++) {
        if (st->cancel_requested) {
            break;
        }

        /* Update progress percentage */
        st->progress = (float)i / (float)total_items;
        snprintf(st->status_text, sizeof(st->status_text), 
                 "Hashing file %d of %d...", i + 1, file_count);
        snprintf(st->main_text, sizeof(st->main_text), "Computing hashes...");
        snprintf(st->sub_text, sizeof(st->sub_text), "%3.0f%% complete", st->progress * 100.0f);
        
        char rel_display[1024];
        path_get_relative(base_path, file_collection[i], rel_display, sizeof(rel_display));
        snprintf(st->sub_sub_text, sizeof(st->sub_sub_text), "%s", rel_display);

        /* Compute hash */
        char computed[129] = {0};
        compute_hash(file_collection[i], st->method, computed);

        /* Populate results */
        snprintf(st->results[i].filename, sizeof(st->results[i].filename), "%s", file_collection[i]);
        snprintf(st->results[i].hash, sizeof(st->results[i].hash), "%s", computed);
        st->results[i].status = 0; /* Unused in generation mode */
        snprintf(st->results[i].method, sizeof(st->results[i].method), "%s", 
                 checksum_method_to_display_name(st->method));
    }

    if (st->cancel_requested) {
        snprintf(st->main_text, sizeof(st->main_text), "Operation cancelled.");
        st->sub_text[0] = '\0';
        st->sub_sub_text[0] = '\0';
        st->status_text[0] = '\0';
        st->busy = 0;
        st->done = 0;
        return;
    }

    /* Final progress update */
    st->progress = 1.0f;
    snprintf(st->sub_text, sizeof(st->sub_text), "100%% complete");

    /* Create digest file */
    char out_filename[256];
    checksum_method_get_output_filename(st->method, out_filename, sizeof(out_filename));
    path_combine(base_path, out_filename, st->output_path, sizeof(st->output_path));

    int rc = output_writer_generate_file(st->output_path, base_path, st->results, file_count, st->method);

    if (rc == 0) {
        snprintf(st->main_text, sizeof(st->main_text), "Wrote checksum file");
        snprintf(st->sub_text, sizeof(st->sub_text), "%s", out_filename);
        snprintf(st->sub_sub_text, sizeof(st->sub_sub_text), "Processed %d files in base path.", file_count);
        snprintf(st->status_text, sizeof(st->status_text), "Wrote %s of %d files using %s", 
                 out_filename, file_count, checksum_method_to_display_name(st->method));
    } else {
        snprintf(st->main_text, sizeof(st->main_text), "Error writing file!");
        snprintf(st->status_text, sizeof(st->status_text), "Could not write %s", st->output_path);
    }

    st->busy = 0;
    st->done = 1;
}

static void run_validate_digests(AppState *st, char **file_collection, int file_count, const char *base_path) {
    st->is_validation = 1;
    st->progress = 0.0f;
    st->busy = 1;
    st->done = 0;
    st->result_count = 0;
    st->results = NULL;
    memset(&st->summary, 0, sizeof(st->summary));

    /* Initialize results array */
    int results_capacity = 128;
    st->results = calloc(results_capacity, sizeof(FileHash));
    if (!st->results) {
        snprintf(st->status_text, sizeof(st->status_text), "Memory allocation failed!");
        st->busy = 0;
        return;
    }

    st->summary.total_digests = file_count;

    for (int fc = 0; fc < file_count; fc++) {
        if (st->cancel_requested) {
            break;
        }

        const char *digest_file = file_collection[fc];
        ChecksumMethod method;
        if (!checksum_method_try_parse_digest_path(digest_file, &method)) {
            continue;
        }

        int total_lines = count_file_lines(digest_file);
        if (total_lines == 0) total_lines = 1;

        /* Extract only the filename from the digest path for display */
        const char *digest_filename = strrchr(digest_file, '/');
#if defined(_WIN32)
        if (!digest_filename) digest_filename = strrchr(digest_file, '\\');
#endif
        if (!digest_filename) digest_filename = digest_file;
        else digest_filename++; /* Skip separation separator */

        snprintf(st->main_text, sizeof(st->main_text), "Checking %s hash", checksum_method_to_display_name(method));
        snprintf(st->status_text, sizeof(st->status_text), "Analyzing files in %s", digest_filename);

        FILE *f = fopen(digest_file, "r");
        if (!f) {
            continue;
        }

        char line[2048];
        int done_lines = 0;
        int hash_len = checksum_method_get_hash_length(method);

        while (fgets(line, sizeof(line), f)) {
            if (st->cancel_requested) {
                break;
            }

            done_lines++;
            st->progress = (float)done_lines / (float)total_lines;
            snprintf(st->sub_text, sizeof(st->sub_text), "%3.0f%% complete", st->progress * 100.0f);

            strip_newline(line);
            int line_len = strlen(line);

            /* Skip empty or comment lines */
            if (line_len == 0) {
                continue;
            }
            if (line[0] == ';' || line[0] == '#') {
                continue;
            }

            if (line_len > hash_len) {
                char filename[1024] = {0};
                char hash_str[129] = {0};

                if (method == METHOD_CRC32) {
                    /* SFV format: filename is on the left, CRC32 is on the right */
                    int filename_len = line_len - hash_len;
                    if (filename_len >= (int)sizeof(filename)) filename_len = sizeof(filename) - 1;
                    strncpy(filename, line, filename_len);
                    filename[filename_len] = '\0';
                    trim_string(filename);

                    strncpy(hash_str, line + filename_len, sizeof(hash_str) - 1);
                    hash_str[sizeof(hash_str) - 1] = '\0';
                    trim_string(hash_str);
                } else {
                    /* Standard formats: Hash is on the left, filename on the right */
                    strncpy(hash_str, line, hash_len);
                    hash_str[hash_len] = '\0';
                    trim_string(hash_str);

                    if (line_len > hash_len + 1) {
                        strncpy(filename, line + hash_len + 1, sizeof(filename) - 1);
                        filename[sizeof(filename) - 1] = '\0';
                        trim_string(filename);
                    } else {
                        filename[0] = '\0';
                    }
                }

                if (is_hex_string(hash_str)) {
                    string_to_upper(hash_str);

                    /* Locate base physical path of file relative to digest folder */
                    char digest_dir[4096];
                    path_get_parent(digest_file, digest_dir, sizeof(digest_dir));

                    char normalized_rel[1024];
                    normalize_digest_path(filename, normalized_rel, sizeof(normalized_rel));

                    char full_file_path[4096];
                    path_combine(digest_dir, normalized_rel, full_file_path, sizeof(full_file_path));

                    int status;
                    if (path_is_file(full_file_path)) {
                        char computed[129] = {0};
                        compute_hash(full_file_path, method, computed);
                        if (strcmp(computed, hash_str) == 0) {
                            status = 101; /* valid */
                        } else {
                            status = 909; /* invalid */
                        }
                    } else {
                        status = 404; /* not found */
                    }

                    /* Grow dynamically allocated results array if we run out of capacity */
                    if (st->result_count >= results_capacity) {
                        results_capacity *= 2;
                        FileHash *new_results = realloc(st->results, results_capacity * sizeof(FileHash));
                        if (!new_results) {
                            /* Out of memory */
                            break;
                        }
                        st->results = new_results;
                    }

                    FileHash *res = &st->results[st->result_count];
                    strncpy(res->filename, filename, sizeof(res->filename));
                    strncpy(res->hash, hash_str, sizeof(res->hash));
                    res->status = status;
                    strncpy(res->method, checksum_method_to_display_name(method), sizeof(res->method));

                    /* Update summary stats */
                    st->summary.total_files++;
                    if (status == 101) st->summary.total_valid++;
                    else if (status == 404) st->summary.total_not_found++;
                    else if (status == 909) st->summary.total_invalid++;

                    st->result_count++;
                }
            }
        }
        fclose(f);
    }

    if (st->cancel_requested) {
        snprintf(st->main_text, sizeof(st->main_text), "Operation cancelled.");
        st->sub_text[0] = '\0';
        st->sub_sub_text[0] = '\0';
        st->status_text[0] = '\0';
        st->busy = 0;
        st->done = 0;
        return;
    }

    st->progress = 1.0f;
    snprintf(st->main_text, sizeof(st->main_text), "");
    snprintf(st->sub_text, sizeof(st->sub_text), "");
    snprintf(st->sub_sub_text, sizeof(st->sub_sub_text), "Done validation!");
    st->status_text[0] = '\0';

    st->busy = 0;
    st->done = 1;
}

void processor_run(AppState *st, char **dropped_paths, int dropped_count) {
    if (!st || !dropped_paths || dropped_count == 0) return;

    /* Clean up any existing results */
    if (st->results) {
        free(st->results);
        st->results = NULL;
    }
    st->result_count = 0;
    st->cancel_requested = 0;
    st->busy = 1;
    st->done = 0;

    /* ── 1. Calculate Base Path ── */
    char shortest_path[4096];
    memset(shortest_path, 'a', 4095);
    shortest_path[4095] = '\0';

    for (int i = 0; i < dropped_count; i++) {
        char parent[4096];
        path_get_parent(dropped_paths[i], parent, sizeof(parent));
        if (strlen(parent) > 0 && strlen(parent) < strlen(shortest_path)) {
            strcpy(shortest_path, parent);
        }
    }

    char base_path[4096];
    if (shortest_path[0] == 'a' && shortest_path[1] == 'a') {
        base_path[0] = '\0';
    } else {
        strcpy(base_path, shortest_path);
    }

    /* ── 2. Expand Inputs ── */
    char **file_collection = NULL;
    int file_count = 0;
    int file_capacity = 0;

    for (int i = 0; i < dropped_count; i++) {
        const char *item = dropped_paths[i];
        if (path_is_directory(item)) {
            path_recursive_list(item, &file_collection, &file_count, &file_capacity);
        } else if (path_is_file(item)) {
            if (file_count >= file_capacity) {
                file_capacity = file_capacity == 0 ? 128 : file_capacity * 2;
                char **new_collection = realloc(file_collection, file_capacity * sizeof(char *));
                if (!new_collection) {
                    /* Out of memory */
                    path_free_list(file_collection, file_count);
                    st->busy = 0;
                    return;
                }
                file_collection = new_collection;
            }
            file_collection[file_count] = strdup(item);
            file_count++;
        }
    }

    /* ── 3. Auto-Detect Mode ── */
    int all_files_are_digest = 1;
    for (int i = 0; i < dropped_count; i++) {
        ChecksumMethod method_dummy;
        if (!checksum_method_try_parse_digest_path(dropped_paths[i], &method_dummy)) {
            all_files_are_digest = 0;
            break;
        }
    }

    if (all_files_are_digest) {
        run_validate_digests(st, file_collection, file_count, base_path);
    } else {
        run_create_digests(st, file_collection, file_count, base_path);
    }

    /* Clean up the file collection strings */
    path_free_list(file_collection, file_count);
}
