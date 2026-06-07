#include "settings.h"
#include "ini.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MKDIR(d) mkdir(d, 0777)
#endif

static void get_settings_path(char *out_path, size_t max_size) {
    const char *home = NULL;
#if defined(_WIN32)
    home = getenv("APPDATA");
    if (home) {
        snprintf(out_path, max_size, "%s\\ChecksumDrop", home);
        MKDIR(out_path);
        snprintf(out_path, max_size, "%s\\ChecksumDrop\\settings.ini", home);
        return;
    }
#else
    home = getenv("HOME");
    if (home) {
        char dir_path[1024];
        /* Try creating ~/.config folder first just in case */
        snprintf(dir_path, sizeof(dir_path), "%s/.config", home);
        MKDIR(dir_path);
        
        snprintf(dir_path, sizeof(dir_path), "%s/.config/ChecksumDrop", home);
        MKDIR(dir_path);
        
        snprintf(out_path, max_size, "%s/.config/ChecksumDrop/settings.ini", home);
        return;
    }
#endif
    /* Fallback */
    snprintf(out_path, max_size, "settings.ini");
}

static int settings_ini_handler(void* user, const char* section, const char* name, const char* value) {
    AppState *st = (AppState*)user;
    if (strcmp(section, "Settings") == 0) {
        if (strcmp(name, "Method") == 0) {
            ChecksumMethod parsed_method;
            if (checksum_method_try_parse_setting(value, &parsed_method)) {
                st->method = parsed_method;
            }
        }
    }
    return 1;
}

void settings_load(AppState *st) {
    char path[1024];
    get_settings_path(path, sizeof(path));
    
    /* Default is MD5 */
    st->method = METHOD_MD5;
    
    ini_parse(path, settings_ini_handler, st);
}

void settings_save(AppState *st) {
    char path[1024];
    get_settings_path(path, sizeof(path));
    
    FILE *f = fopen(path, "w");
    if (!f) return;
    
    fprintf(f, "[Settings]\n");
    fprintf(f, "Method = %s\n", checksum_method_to_token(st->method));
    fclose(f);
}
