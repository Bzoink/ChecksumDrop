#pragma once

/*
 * x11_dnd.h — Linux/X11 drag-and-drop helper
 *
 * On compositing window managers (e.g. Cinnamon/Mutter), the WM reparents
 * every application window inside its own "frame" window.  When a
 * drag-and-drop source (e.g. Nemo) scans the window hierarchy for an XDND
 * target, it lands on the WM frame first.  Because the frame has no
 * XdndAware property, the source assumes the window does not support XDND
 * and never sends any drag events — so drops silently do nothing.
 *
 * The XDND protocol has a proxy mechanism for exactly this case:
 *   • Set XdndAware on the frame window (version 5).
 *   • Set XdndProxy on the frame window pointing to the real app window.
 *
 * XDND-compliant sources (GTK3/Nemo included) will then forward all drag
 * events to our app window, where GLFW's existing handler processes them
 * normally.
 *
 * Call xdnd_proxy_setup() once, immediately after raylib's InitWindow().
 */

#if defined(__linux__)
void xdnd_proxy_setup(void *glfw_win_handle);
#endif
