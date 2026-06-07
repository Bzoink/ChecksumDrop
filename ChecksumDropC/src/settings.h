#pragma once

#include "app_state.h"

/* Loads settings from the user's config directory (~/.config/ChecksumDrop/settings.ini or %APPDATA%/ChecksumDrop/settings.ini).
 * If the settings file does not exist, sets defaults (MD5). */
void settings_load(AppState *st);

/* Saves the current settings (st->method) to the user's config directory, creating folders if necessary. */
void settings_save(AppState *st);
