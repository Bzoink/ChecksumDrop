#include "output_writer.h"
#include "path_utils.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

int output_writer_generate_file(const char *output_filepath,
                                const char *base_path,
                                const FileHash *hashes,
                                int count,
                                ChecksumMethod method) {
    if (!output_filepath || !hashes) return -1;

    /* Open in standard text-translation mode to use system-native line endings */
    FILE *f = fopen(output_filepath, "w");
    if (!f) return -1;

    /* Get current date/time */
    time_t raw_time;
    time(&raw_time);
    struct tm *info = localtime(&raw_time);
    char date_str[64];
    /* Ambicultural-friendly format similar to .NET's general date-time */
    strftime(date_str, sizeof(date_str), "%m/%d/%Y %H:%M:%S", info);

    /* Header comment line */
    fprintf(f, "; Created with ChecksumDrop version 1.1 %s using %s hash\n",
            date_str, checksum_method_to_token(method));

    /* File lines */
    char rel_path[1024];
    for (int i = 0; i < count; i++) {
        path_get_relative(base_path, hashes[i].filename, rel_path, sizeof(rel_path));

        if (method == METHOD_CRC32) {
            /* Path first, then two spaces, then hash (SFV convention) */
            fprintf(f, "%s  %s\n", rel_path, hashes[i].hash);
        } else {
            /* Hash first, then two spaces, then path (md5sum convention) */
            fprintf(f, "%s  %s\n", hashes[i].hash, rel_path);
        }
    }

    fclose(f);
    return 0;
}
