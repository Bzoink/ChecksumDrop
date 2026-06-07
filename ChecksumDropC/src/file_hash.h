#pragma once

typedef struct {
    char filename[1024];   /* original dropped/recursed filepath or relative path */
    char hash[129];        /* uppercase hex hash, max 128 chars + NUL */
    int  status;           /* 0 for unset, 101 for OK, 404 for not found, 909 for mismatch */
    char method[32];       /* method display name (e.g. "SHA3-256") */
} FileHash;
