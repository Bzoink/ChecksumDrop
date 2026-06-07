#pragma once
#include "app_state.h"

/* Runs the core file expansion, base path calculation, mode detection,
 * and executes either the Generation (Phase 2) or Validation (Phase 3) path.
 *
 * Parameters:
 *   - `st`: Pointer to global `AppState` structure to populate progress / results / outputs.
 *   - `dropped_paths`: list of file and folder paths dropped onto the window.
 *   - `dropped_count`: number of paths in `dropped_paths`.
 */
void processor_run(AppState *st, char **dropped_paths, int dropped_count);
