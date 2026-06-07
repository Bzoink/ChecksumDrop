#pragma once
#include "checksum_method.h"
#include "file_hash.h"

/* Formats the digest file text into an allocated buffer and writes it out.
 * Parameters:
 *   - `output_filepath`: absolute target path where the checksum file is to be written.
 *   - `base_path`: the calculated shortest parent directory (base directory).
 *   - `hashes`: array of `FileHash` containing final computed hashes and original full paths.
 *   - `count`: total number of files.
 *   - `method`: configured checksum method.
 *
 * Returns 0 on success, or -1 if the output file cannot be created.
 */
int output_writer_generate_file(const char *output_filepath,
                                const char *base_path,
                                const FileHash *hashes,
                                int count,
                                ChecksumMethod method);
